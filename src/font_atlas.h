#ifndef FONT_ATLAS_H
#define FONT_ATLAS_H
#include <vector>
#include "base64.h"
#include "utils.h"
class FontAtlas {
public:
  std::map<char, CharacterEntry> entries;
  std::map<int, std::vector<float>> linesCache;
  std::map<int, std::string> contentCache;
  GLuint texture_id;
  FT_UInt atlas_width, atlas_height, smallest_top;
  uint32_t fs;
  FT_Library ft;
  bool wasGenerated = false;
  FT_Face face;
  std::string decoded;
  RenderChar render(char c, float x = 0.0, float y = 0.0, Vec4f color = vec4fs(1)) {
    auto entry = entries[c];
    RenderChar r;
    float x2 = x + entry.left;
    float y2 = y - entry.top + (atlas_height - smallest_top);
    r.pos = vec2f(x2, -y2);
    r.size = vec2f(entry.width, -entry.height);
    r.uv_pos = vec2f(entry.offset, 0.0f);
    r.uv_size = vec2f(entry.width / (float) atlas_width, entry.height / atlas_height);
    r.fg_color = color;
    return r;
  }
  float getAdvance(char c) {
    return entries[c].advance;
  }
  FontAtlas(std::string path, uint32_t fontSize) {
    if (FT_Init_FreeType(&ft)) {
        std::cout << "ERROR::FREETYPE: Could not init FreeType Library" << std::endl;
        return;
    }
//    std::string d = Base64::decode(content, decoded);
    readFont(path, fontSize);
  }
  void readFont(std::string path, uint32_t fontSize) {
    if(wasGenerated) {
       FT_Done_Face(face);
    }
    decoded = file_to_string(path);
    if (FT_New_Memory_Face(ft, (const FT_Byte *) decoded.c_str(), decoded.length(), 0, &face)) {
      std::cout << "ERROR::FREETYPE: Failed to load font" << std::endl;
      return;
    }
    renderFont(fontSize);
  }
  void renderFont(uint32_t fontSize) {
   if(wasGenerated) {
      glDeleteTextures(1, &texture_id);
      entries.clear();
      linesCache.clear();
    }   
    fs = fontSize;
    atlas_width = 0;
    atlas_height = 0;
    smallest_top = 1e9;    
    FT_Set_Pixel_Sizes(face, 0, fontSize);
    // TODO should this be here?
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    for(int i = 0; i < 128; i++) {
      if (FT_Load_Char(face, i, FT_LOAD_RENDER)) {
        std::cout << "Failed to load char: " << (char) i << "\n";
        return;
      }
      auto bm = face->glyph->bitmap;
      atlas_width += bm.width;
      atlas_height = bm.rows > atlas_height ? bm.rows : atlas_height;

    }


    // texture
    glActiveTexture(GL_TEXTURE0);
    glGenTextures(1, &texture_id);
    glBindTexture(GL_TEXTURE_2D, texture_id);

    //params
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, (GLsizei) atlas_width, (GLsizei) atlas_height, 0, GL_RED, GL_UNSIGNED_BYTE, nullptr);
    int  xOffset = 0;
    for(int i = 0; i < 128; i++) {
      if (FT_Load_Char(face, i, FT_LOAD_RENDER)) {
        std::cout << "Failed to load char: " << (char) i << "\n";
        return;
      }
      CharacterEntry entry;
      auto bm = face->glyph->bitmap;
      entry.width = bm.width;
      entry.height = bm.rows;
      entry.top = face->glyph->bitmap_top;
      entry.left = face->glyph->bitmap_left;
      entry.advance = face->glyph->advance.x >> 6;
//      entry.buffer = bm.buffer;
      entry.c = (char)i;
      if(smallest_top == 0 && entry.top > 0)
        smallest_top = entry.top;
      else
        smallest_top = entry.top < smallest_top && entry.top != 0 ? entry.top : smallest_top;
      entries.insert(std::pair<char, CharacterEntry>(entry.c, entry));


      entries[(char)i].offset = (float) xOffset / (float) atlas_width;
      glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
      glTexSubImage2D(
                      GL_TEXTURE_2D,
                      0,
                      xOffset,
                      0,
                      entry.width,
                      entry.height,
                      GL_RED,
                      GL_UNSIGNED_BYTE,
                      face->glyph->bitmap.buffer);

      xOffset += entry.width;

   }    
   wasGenerated = true;
  }
  float getAdvance(std::string line) {
    float v = 0;
    std::string::const_iterator c;
    for (c = line.begin(); c != line.end(); c++) {
      v += entries[*c].advance;
    }
    return v;
  }

  std::vector<float>* getAllAdvance(std::string line, int y) {
    if (linesCache.count(y)) {
      if(contentCache[y] == line) {
        return &linesCache[y];
      }
    }
    std::vector<float> values;
    std::string::const_iterator c;
    for (c = line.begin(); c != line.end(); c++) {
      values.push_back(entries[*c].advance);
    }
    linesCache[y] = values;
    contentCache[y] = line;
    return &linesCache[y];
  }

};

#endif
