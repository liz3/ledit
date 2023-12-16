#ifndef FONT_ATLAS_H
#define FONT_ATLAS_H
#include <string>
#include <vector>
#include <filesystem>
#include "base64.h"
#include "freetype/freetype.h"
#include "freetype/fttypes.h"
#include "shader.h"
#include "utils.h"
#include "glad.h"
#include "utf8String.h"

namespace fs = std::filesystem;

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
  std::vector<Utf8String> errors;
  FT_UInt atlas_width, atlas_height, atlas_height_absolute, smallest_top,
      atlas_height_original;
  uint32_t fs, virtual_fs;
  FT_Library ft;
  bool wasGenerated = false;
  float xOffset = 0;
  uint8_t tabWidth = 2;
  float scale = 1;
  std::vector<FontFace *> faces;
  RenderChar render(char32_t c, float x = 0.0, float y = 0.0,
                    Vec4f color = vec4fs(1)) {
    if (c >= 128 || c < 32)
      lazyLoad(c);
    auto *entry = &entries[c];
    RenderChar r;
    float x2 = x + entry->left * scale;
    float y2 = y + atlas_height;
    y2 -= ((entry->top) * scale);
    if (entry->hasColor) {
      float height = entry->height * (fs / entry->height);
      y2 += ((entry->top) - ((height) - (fs)*0.15)) * scale;
    }
    r.pos = vec2f(x2, -y2);
    if (entry->hasColor) {
      float height = entry->height * (fs / entry->height);
      r.size = vec2f(((float)fs) * scale, (-height) * scale);

    } else
      r.size = vec2f(entry->width * scale, (-entry->height) * scale);
    r.uv_pos = vec2f(entry->offset, 0.0f);
    r.uv_size = vec2f(entry->width / (float)atlas_width,
                      entry->height / (float)atlas_height_absolute);
    r.fg_color = color;
    r.hasColor = entry->hasColor ? 1 : 0;
    return r;
  }
  float getColonWidth() { return virtual_fs * scale; }
  void ensureTab() {
    if (entries.count(U'\t'))
      return;
    CharacterEntry entry;
    entry.advance = entries[U' '].advance * tabWidth;
    entry.hasColor = false;
    entry.width = 0;
    entry.height = 0;
    entries[U'\t'] = entry;
  }
  float getAdvance(char32_t c) {
    if (c >= 128 || c < 32)
      lazyLoad(c);
    return entries[c].advance * scale;
  }
  FontAtlas(std::string path, uint32_t fontSize) {
    errors.clear();
    if (FT_Init_FreeType(&ft)) {
      std::cout << "ERROR::FREETYPE: Could not init FreeType Library"
                << std::endl;
      return;
    }
    virtual_fs = fontSize;
    if (fontSize % 5 != 0)
      fontSize -= (fontSize % 5);
    if (fontSize < 15)
      fontSize = 15;
    readFont(path, fontSize, true);
  }
  size_t fontSelectSize(uint32_t size, FT_Face face) {
    size_t i;
    for (i = 0; i < face->num_fixed_sizes; i++) {
      auto entry = face->available_sizes[i];
      if (entry.height >= size) {
        return i;
      }
    }
    return i;
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
    {
      fs::path p(path);
      if (p.extension().generic_string() == ".ttc" && !shouldRender) {
        FT_Face gFace;
        FT_Error error = FT_New_Face(ft, path.c_str(), -1, &gFace);
        if (error) {
          errors.push_back(U"Failed to load ttc " + Utf8String(path));
          return;
        }
        int numFaces = gFace->num_faces;

        for (int i = 0; i < numFaces; ++i) {
          FontFace *face = new FontFace();
          error = FT_New_Face(ft, path.c_str(), i, &face->face);
          if (error) {
            errors.push_back(U"Failed to load ttc " + Utf8String(path) + U" [" +
                             std::to_string(i) + U"]");
            delete face;
            continue;
          }
          FT_Bool isItalic = face->face->style_flags & FT_STYLE_FLAG_ITALIC;
          FT_Bool isBold = face->face->style_flags & FT_STYLE_FLAG_BOLD;
          if (isBold || isItalic) {
            FT_Done_Face(face->face);
            delete face;
            continue;
          }
          face->path = path;
          face->hasColor = isColorEmojiFont(face->face);
          if (face->hasColor)
            FT_Select_Size(face->face, fontSelectSize(fontSize, face->face));
          else
            FT_Set_Pixel_Sizes(face->face, 0, fontSize);
          faces.push_back(face);
        }
        FT_Done_Face(gFace);
        return;
      }
    }
    FontFace *face = new FontFace();
    int x = FT_New_Face(ft, path.c_str(), 0, &face->face);
    if (x) {
      errors.push_back(U"Failed to load ttf " + Utf8String(path));
      return;
    }
    face->path = path;
    face->hasColor = isColorEmojiFont(face->face);
    if (face->hasColor)
      FT_Select_Size(face->face, fontSelectSize(fontSize, face->face));
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
  void changeScale(int diff) {

    if ((virtual_fs <= 15 && diff == -1) || (virtual_fs >= 100 && diff == 1))
      return;

    virtual_fs += diff;
    scale = (float)virtual_fs / (float)fs;

    if ((diff == -1 && scale < 0.6 && fs > 20) ||
        (scale > 2.5 && diff == 1 && fs < 35)) {
      auto newSize = virtual_fs < 20 ? 20 : virtual_fs > 35 ? 35 : virtual_fs;
      for (auto &face : faces) {
        if (face->hasColor)
          FT_Select_Size(face->face, fontSelectSize(newSize, face->face));
        else
          FT_Set_Pixel_Sizes(face->face, 0, newSize);
      }
      renderFont(newSize, faces[0]);
      scale = (float)virtual_fs / (float)fs;
    }

    atlas_height = atlas_height_original * scale;
  }
  void renderFont(uint32_t fontSize, FontFace *faceEntry) {
    if (wasGenerated) {
           glBindTexture(GL_TEXTURE_2D, 0);
      glDeleteTextures(1, &texture_id);
      linesCache.clear();
            entries.clear();
            contentCache.clear();

    }
    auto face = faceEntry->face;
    fs = fontSize;
    atlas_width = 0;
    atlas_height = 0;
    smallest_top = 1e9;
    for (int i = 32; i < 128; i++) {
      if (FT_Load_Char(face, i,
                       FT_LOAD_RENDER | FT_LOAD_TARGET_(FT_RENDER_MODE_SDF))) {
        std::cout << "Failed to load char: " << (char)i << "\n";
        return;
      }


      auto bm = face->glyph->bitmap;
      atlas_width += bm.width;
      atlas_height = bm.rows > atlas_height ? bm.rows : atlas_height;

      CharacterEntry entry;
      entry.width = bm.width;
      entry.height = bm.rows;
      entry.top = face->glyph->bitmap_top;
      entry.left = face->glyph->bitmap_left;
      entry.advance = face->glyph->advance.x >> 6;
      entry.advanceY = face->glyph->advance.y >> 6;
      entry.hasColor = faceEntry->hasColor;
      entry.c = (char32_t)i;
      entry.data = new uint8_t[(int)entry.width * (int)entry.height * 4];
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
    }
    atlas_height_absolute = atlas_height;
    atlas_height_original = atlas_height;
    atlas_height *= scale;

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
    for (int i = 32; i < 128; i++) {
      CharacterEntry &entry = entries[i];
      entry.offset = (float)xOffset / (float)atlas_width;
      entry.xPos = xOffset;

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
    if (c == U'\t') {
      ensureTab();
      return;
    }
    FontFace *faceEntry = nullptr;
    for (auto *e : faces) {
      uint32_t glyph_index = FT_Get_Char_Index(e->face, c);
      if (glyph_index != 0) {
        faceEntry = e;
        break;
      }
    }
    if (!faceEntry) {
      Utf8String errStr = U"No font file for char ";
      errStr += std::to_string((int)c);
      errors.push_back(errStr);
      return;
    }
    auto &face = faceEntry->face;
    auto f = FT_LOAD_RENDER;
    if (faceEntry->hasColor)
      f |= FT_LOAD_COLOR;
    else
      f |= FT_LOAD_TARGET_(FT_RENDER_MODE_SDF);
    FT_Error err = FT_Load_Char(face, c, f);
    if (err) {
      Utf8String errStr = U"No font file for char ";
      errStr += c;
      errors.push_back(errStr);
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
    if (!entry.hasColor) {
      atlas_height = atlas_height_absolute;
      atlas_height_original = atlas_height;
      atlas_height *= scale;
    }
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
    auto vec = line.getCodePoints();
    for (auto c : vec) {
      if (c >= 128 || c < 32)
        lazyLoad(c);
      v += entries[c].advance * scale;
    }
    return v;
  }
  float getAdvance(std::string line) {
    float v = 0;
    std::string::const_iterator c;
    for (c = line.begin(); c != line.end(); c++) {
      char32_t cc = (char32_t)(*c);
      v += entries[cc].advance * scale;
    }
    return v;
  }

  std::vector<float> *getAllAdvance(Utf8String &line, int y) {
    if (linesCache.count(y)) {
      if (contentCache[y] == line) {
        return &linesCache[y];
      }
    }
    std::vector<float> values;
    auto vec = line.getCodePoints();
    Utf8String::const_iterator c;
    for (const auto c : vec) {
      if (c >= 128 || c < 32)
        lazyLoad(c);

      values.push_back(entries[c].advance * scale);
    }
    linesCache[y] = values;
    contentCache[y] = line;
    return &linesCache[y];
  }
  bool isColorEmojiFont(FT_Face &face) { return FT_HAS_COLOR(face); }
};

#endif
