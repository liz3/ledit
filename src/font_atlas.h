#ifndef FONT_ATLAS_H
#define FONT_ATLAS_H
#include <vector>
#include "base64.h"
#include "utils.h"
#include "glad.h"
#include "utf8String.h"

struct FontFace {
  bool hasColor = false;
  std::string path = "";
  FT_Face face;
};
class FontAtlas {
public:
  std::map<char32_t, CharacterEntry> entries;
  std::map<int, std::vector<float>> linesCache;
  std::map<int, Utf8String> contentCache;
  GLuint texture_id;
  FT_UInt atlas_width, atlas_height, atlas_height_absolute, smallest_top;
  uint32_t fs;
  FT_Library ft;
  bool wasGenerated = false;
  int xOffset = 0;
  std::vector<FontFace *> faces;
  RenderChar render(char32_t c, float x = 0.0, float y = 0.0,
                    Vec4f color = vec4fs(1)) {
    if (c >= 128)
      lazyLoad(c);

    auto *entry = &entries[c];
    RenderChar r;
    float x2 = x + entry->left;
    float y2 = y - entry->top + (atlas_height);
    if (entry->hasColor) {
      float height = entry->height * (fs / entry->height);
      y2 += (entry->top) - ((height) - (fs)*0.15);
    }
    r.pos = vec2f(x2, -y2);
    if (entry->hasColor) {
      float height = entry->height * (fs / entry->height);
      r.size = vec2f(((float)fs), (-height));

    } else
      r.size = vec2f(entry->width, -entry->height);
    r.uv_pos = vec2f(entry->offset, 0.0f);
    r.uv_size = vec2f(entry->width / (float)atlas_width,
                      entry->height / atlas_height_absolute);
    r.fg_color = color;
    r.hasColor = entry->hasColor ? 1 : 0;
    return r;
  }
  float getAdvance(char32_t c) { return entries[c].advance; }
  FontAtlas(std::string path, uint32_t fontSize) {
    if (FT_Init_FreeType(&ft)) {
      std::cout << "ERROR::FREETYPE: Could not init FreeType Library"
                << std::endl;
      return;
    }
    readFont(path, fontSize, true);
  }
  void readFont(std::string path, uint32_t fontSize,
                bool shouldRender = false) {
    if (wasGenerated && shouldRender) {
      for (auto *entry : faces) {
        FT_Done_Face(entry->face);
        delete entry;
      }
      faces.clear();
    }
    FontFace *face = new FontFace();
    int x = FT_New_Face(ft, path.c_str(), 0, &face->face);
    if (x) {
      std::cout << "ERROR::FREETYPE: Failed to load font " << x << std::endl;
      return;
    }
    face->path = path;
    face->hasColor = isColorEmojiFont(face->face);
    if (face->hasColor)
      FT_Select_Size(face->face, 0);
    else
      FT_Set_Pixel_Sizes(face->face, 0, fontSize);
    faces.push_back(face);
    if (shouldRender)
      renderFont(fontSize, face);
  }
  void resizeFonts(uint32_t size) {
    for (auto &face : faces) {
      if (face->hasColor)
        FT_Select_Size(face->face, 0);
      else
        FT_Set_Pixel_Sizes(face->face, 0, size);
    }
    fs = size;
  }
  void renderFont(uint32_t fontSize, FontFace *faceEntry) {
    if (wasGenerated) {
      glDeleteTextures(1, &texture_id);
      for (std::map<char32_t, CharacterEntry>::iterator it = entries.begin();
           it != entries.end(); ++it) {
        delete[] it->second.data;
      }
      entries.clear();
      linesCache.clear();
    }
    auto &face = faceEntry->face;
    fs = fontSize;
    atlas_width = 0;
    atlas_height = 0;
    smallest_top = 1e9;
    // TODO should this be here?
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    for (int i = 0; i < 128; i++) {
      if (FT_Load_Char(face, i, FT_LOAD_RENDER)) {
        std::cout << "Failed to load char: " << (char)i << "\n";
        return;
      }
      auto bm = face->glyph->bitmap;
      atlas_width += bm.width;
      atlas_height = bm.rows > atlas_height ? bm.rows : atlas_height;
    }
    atlas_width *= 2;
    atlas_height_absolute = atlas_height;

    // texture
    glActiveTexture(GL_TEXTURE0);
    glGenTextures(1, &texture_id);
    glBindTexture(GL_TEXTURE_2D, texture_id);

    // params
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, (GLsizei)atlas_width,
                 (GLsizei)atlas_height_absolute, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                 nullptr);
    xOffset = 0;
    for (int i = 0; i < 128; i++) {
      if (FT_Load_Char(face, i, FT_LOAD_RENDER)) {
        std::cout << "Failed to load char: " << (char)i << "\n";
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
      entry.hasColor = faceEntry->hasColor;
      entry.c = (char32_t)i;
      (&entry)->data = new uint8_t[(int)entry.width * (int)entry.height * 4];
      for (size_t i = 0; i < (entry.width * entry.height); i++) {
        auto target_index = i * 4;
        entry.data[target_index] = face->glyph->bitmap.buffer[i];
        entry.data[target_index + 1] = face->glyph->bitmap.buffer[i];
        entry.data[target_index + 2] = face->glyph->bitmap.buffer[i];
        entry.data[target_index + 3] = 255;
      }
      if (smallest_top == 0 && entry.top > 0)
        smallest_top = entry.top;
      else
        smallest_top = entry.top < smallest_top && entry.top != 0
                           ? entry.top
                           : smallest_top;
      entries.insert(std::pair<char32_t, CharacterEntry>(entry.c, entry));

      entries[(char32_t)i].offset = (float)xOffset / (float)atlas_width;
      glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
      glTexSubImage2D(GL_TEXTURE_2D, 0, xOffset, 0, entry.width, entry.height,
                      GL_RGBA, GL_UNSIGNED_BYTE, entry.data);

      xOffset += entry.width;
    }
    wasGenerated = true;
  }
  void lazyLoad(char32_t c) {
    if (entries.count(c))
      return;
    FontFace *faceEntry = nullptr;
    for (auto *e : faces) {
      uint32_t glyph_index = FT_Get_Char_Index(e->face, c);
      if (glyph_index != 0) {
        faceEntry = e;
        break;
      }
    }
    if (!faceEntry) {
      return;
    }
    auto &face = faceEntry->face;
    auto f = FT_LOAD_RENDER;
    if (faceEntry->hasColor)
      f |= FT_LOAD_COLOR;
    if (FT_Load_Char(face, c, f)) {
      std::cout << "Failed to load char: " << (char32_t)c << "\n";
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
    entry.hasColor = faceEntry->hasColor;
    if (entry.hasColor)
      entry.advance = fs;
    auto old_width = atlas_width;
    auto old_height = atlas_height;
    atlas_width += bm.width;
    atlas_height_absolute =
        bm.rows > atlas_height_absolute ? bm.rows : atlas_height_absolute;
    if (!entry.hasColor)
      atlas_height = atlas_height_absolute;
    entry.offset = (float)xOffset / (float)atlas_width;
    GLuint new_tex_id;
    glGenTextures(1, &new_tex_id);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, new_tex_id);
    // params
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, (GLsizei)atlas_width,
                 (GLsizei)atlas_height_absolute, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                 nullptr);
    //   glCopyImageSubData(texture_id, GL_TEXTURE_2D, 0, 0, 0, 0, new_tex_id,
    //   GL_TEXTURE_2D, 0, 0, 0, 0, old_width, old_height, 0);
    //   glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0,0,0,0, old_width, old_height);
    //   xOffset = 0;
    for (std::map<char32_t, CharacterEntry>::iterator it = entries.begin();
         it != entries.end(); ++it) {
      it->second.offset = (float)it->second.xPos / (float)atlas_width;
      glTexSubImage2D(GL_TEXTURE_2D, 0, it->second.xPos, 0, it->second.width,
                      it->second.height, GL_RGBA, GL_UNSIGNED_BYTE,
                      it->second.data);
    }
    (&entry)->data = new uint8_t[(int)entry.width * (int)entry.height * 4];
    if (entry.hasColor) {
      for (size_t i = 0; i < ((int)entry.width * (int)entry.height) * 4;
           i += 4) {
        entry.data[i + 2] = face->glyph->bitmap.buffer[i];
        entry.data[i + 1] = face->glyph->bitmap.buffer[i + 1];
        entry.data[i] = face->glyph->bitmap.buffer[i + 2];
        entry.data[i + 3] = face->glyph->bitmap.buffer[i + 3];
      }
    } else {
      for (size_t i = 0; i < (entry.width * entry.height); i++) {
        auto target_index = i * 4;
        entry.data[target_index] = face->glyph->bitmap.buffer[i];
        entry.data[target_index + 1] = face->glyph->bitmap.buffer[i];
        entry.data[target_index + 2] = face->glyph->bitmap.buffer[i];
        entry.data[target_index + 3] = 255;
      }
    }
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexSubImage2D(GL_TEXTURE_2D, 0, xOffset, 0, entry.width, entry.height,
                    GL_RGBA, GL_UNSIGNED_BYTE, entry.data);
    glDeleteTextures(1, &texture_id);
    texture_id = new_tex_id;
    entry.c = (char32_t)c;
    xOffset += entry.width;
    entries.insert(std::pair<char32_t, CharacterEntry>(entry.c, entry));
  }
  float getAdvance(Utf8String line) {
    float v = 0;
    Utf8String::const_iterator c;
    for (c = line.begin(); c != line.end(); c++) {
      if (*c >= 128)
        lazyLoad(*c);
      v += entries[*c].advance;
    }
    return v;
  }
  float getAdvance(std::string line) {
    float v = 0;
    std::string::const_iterator c;
    for (c = line.begin(); c != line.end(); c++) {
      char16_t cc = (char16_t)(*c);
      v += entries[cc].advance;
    }
    return v;
  }

  std::vector<float> *getAllAdvance(Utf8String line, int y) {
    if (linesCache.count(y)) {
      if (contentCache[y] == line) {
        return &linesCache[y];
      }
    }
    std::vector<float> values;
    Utf8String::const_iterator c;
    for (c = line.begin(); c != line.end(); c++) {
      if (*c >= 128)
        lazyLoad(*c);

      values.push_back(entries[*c].advance);
    }
    linesCache[y] = values;
    contentCache[y] = line;
    return &linesCache[y];
  }
  bool isColorEmojiFont(FT_Face &face) {
    static const uint32_t tag = FT_MAKE_TAG('C', 'B', 'D', 'T');
    unsigned long length = 0;
    FT_Load_Sfnt_Table(face, tag, 0, nullptr, &length);
    if (length)
      return true;

    return false;
  }
};

#endif
