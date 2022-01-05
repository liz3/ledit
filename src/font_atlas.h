#ifndef FONT_ATLAS_H
#define FONT_ATLAS_H
#include <vector>
#include "base64.h"
#include "utils.h"
#include "glad.h"
class FontAtlas {
public:
  std::map<char16_t, CharacterEntry> entries;
  std::map<int, std::vector<float>> linesCache;
  std::map<int, std::u16string> contentCache;
  GLuint texture_id;
  FT_UInt atlas_width, atlas_height, smallest_top;
  uint32_t fs;
  FT_Library ft;
  bool wasGenerated = false;
  FT_Face face;
  int xOffset = 0;
  RenderChar render(char16_t c, float x = 0.0, float y = 0.0, Vec4f color = vec4fs(1)) {
    if(c >= 128)
      lazyLoad(c);

    auto* entry = &entries[c];
    RenderChar r;
    float x2 = x + entry->left;
    float y2 = y - entry->top + (atlas_height);
    r.pos = vec2f(x2, -y2);
    r.size = vec2f(entry->width, -entry->height);
    r.uv_pos = vec2f(entry->offset, 0.0f);
    r.uv_size = vec2f(entry->width / (float) atlas_width, entry->height / atlas_height);
    r.fg_color = color;
    return r;
  }
  float getAdvance(char16_t c) {
    return entries[c].advance;
  }
  FontAtlas(std::string path, uint32_t fontSize) {
    if (FT_Init_FreeType(&ft)) {
        std::cout << "ERROR::FREETYPE: Could not init FreeType Library" << std::endl;
        return;
    }
    readFont(path, fontSize);
  }
  void readFont(std::string path, uint32_t fontSize) {
    if(wasGenerated) {
       FT_Done_Face(face);
    }
    int x = FT_New_Face(ft,  path.c_str(), 0, &face);
    if (x) {
      std::cout << "ERROR::FREETYPE: Failed to load font " << x << std::endl;
      return;
    }
    renderFont(fontSize);
  }
  void renderFont(uint32_t fontSize) {
   if(wasGenerated) {
      glDeleteTextures(1, &texture_id);
    for(std::map<char16_t, CharacterEntry>::iterator it = entries.begin(); it != entries.end(); ++it) {
      delete[] it->second.data;
    }
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
    atlas_width *= 2;

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
     xOffset = 0;
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
      entry.xPos = xOffset;
      entry.c = (char16_t)i;
      (&entry)->data = new uint8_t[(int)entry.width * (int)entry.height];
      memcpy(entry.data, face->glyph->bitmap.buffer, entry.width * entry.height);
      if(smallest_top == 0 && entry.top > 0)
        smallest_top = entry.top;
      else
        smallest_top = entry.top < smallest_top && entry.top != 0 ? entry.top : smallest_top;
      entries.insert(std::pair<char16_t, CharacterEntry>(entry.c, entry));


      entries[(char16_t)i].offset = (float) xOffset / (float) atlas_width;
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
  void lazyLoad(char16_t c) {
    if(entries.count(c))
      return;
    if (FT_Load_Char(face, c, FT_LOAD_RENDER)) {
      std::cout << "Failed to load char: " << (char) c << "\n";
      return;
    }
    CharacterEntry entry;
    auto bm = face->glyph->bitmap;
    entry.width = bm.width;
    entry.height = bm.rows;
    entry.top = face->glyph->bitmap_top;
    entry.left = face->glyph->bitmap_left;
    entry.advance = face->glyph->advance.x >> 6;
    entry.xPos = xOffset;
    auto old_width = atlas_width;
    auto old_height = atlas_height;
    atlas_width += bm.width;
    atlas_height = bm.rows > atlas_height ? bm.rows : atlas_height;
    entry.offset = (float) xOffset / (float) atlas_width;
    GLuint new_tex_id;
    glGenTextures(1, &new_tex_id);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, new_tex_id);
    //params
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

   glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

   glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, (GLsizei) atlas_width , (GLsizei) atlas_height, 0, GL_RED, GL_UNSIGNED_BYTE, nullptr);
//   glCopyImageSubData(texture_id, GL_TEXTURE_2D, 0, 0, 0, 0, new_tex_id, GL_TEXTURE_2D, 0, 0, 0, 0, old_width, old_height, 0);
//   glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0,0,0,0, old_width, old_height);
//   xOffset = 0;
    for(std::map<char16_t, CharacterEntry>::iterator it = entries.begin(); it != entries.end(); ++it) {
      it->second.offset = (float) it->second.xPos / (float)atlas_width;
      glTexSubImage2D(
                     GL_TEXTURE_2D,
                    0,
                    it->second.xPos,
                    0,
                    it->second.width,
                    it->second.height,
                    GL_RED,
                    GL_UNSIGNED_BYTE,
                    it->second.data);

    }


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
    glDeleteTextures(1, &texture_id);
    texture_id = new_tex_id;
    entry.c = (char16_t)c;
    (&entry)->data = new uint8_t[(int)entry.width * (int)entry.height];
    memcpy(entry.data, face->glyph->bitmap.buffer, entry.width * entry.height);
    xOffset += entry.width;
    entries.insert(std::pair<char16_t, CharacterEntry>(entry.c, entry));
  }
  float getAdvance(std::u16string line) {
    float v = 0;
    std::u16string::const_iterator c;
    for (c = line.begin(); c != line.end(); c++) {
      if(*c >= 128)
        lazyLoad(*c);
      v += entries[*c].advance;
    }
    return v;
  }
  float getAdvance(std::string line) {
    float v = 0;
    std::string::const_iterator c;
    for (c = line.begin(); c != line.end(); c++) {
      char16_t cc = (char16_t) (*c);
      v += entries[cc].advance;
    }
    return v;
  }

  std::vector<float>* getAllAdvance(std::u16string line, int y) {
    if (linesCache.count(y)) {
      if(contentCache[y] == line) {
        return &linesCache[y];
      }
    }
    std::vector<float> values;
    std::u16string::const_iterator c;
    for (c = line.begin(); c != line.end(); c++) {
      if(*c >= 128)
        lazyLoad(*c);

      values.push_back(entries[*c].advance);
    }
    linesCache[y] = values;
    contentCache[y] = line;
    return &linesCache[y];
  }

};

#endif
