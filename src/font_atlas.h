#ifndef FONT_ATLAS_H
#define FONT_ATLAS_H
#include <string>
#include <vector>
#include "freetype/freetype.h"
#include "freetype/fttypes.h"
#include "glad.h"
#include "shader.h"
#include "utf8String.h"
#include <map>


// How a face stores its colour artwork.  FT_HAS_COLOR() cannot tell these
// apart -- sfobjs.c raises FT_FACE_FLAG_COLOR for CBLC, 'sbix', 'COLR' and
// 'SVG' alike -- but they need completely different handling, so classify the
// face once at load time instead.
enum class ColorKind {
  None,         // monochrome outlines
  Strike,       // 'sbix' / CBDT+CBLC pre-rendered bitmaps, sized by strike
  ScalableCOLR, // 'COLR' v0 layers, composited into BGRA by FreeType itself
};

struct FontFace {
  ColorKind kind = ColorKind::None;
  std::string path = "";
  FT_Face face;
  bool hasColor() const { return kind != ColorKind::None; }
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
                    Vec4f color = vec4fs(1));
  float getColonWidth();
  void ensureTab();
  float getAdvance(char32_t c);
  FontAtlas(std::string path, uint32_t fontSize);
  size_t fontSelectSize(uint32_t size, FT_Face face);
  FT_Error setFaceSize(FontFace *entry, uint32_t px);
  void readFont(std::string path, uint32_t fontSize,
                bool shouldRender = false);
  void resizeFonts(uint32_t size);
  void changeScale(int diff);
  void renderFont(uint32_t fontSize, FontFace *faceEntry);
  void lazyLoad(char32_t c);
  float getAdvance(Utf8String line);
  float getAdvance(std::string line);

  std::vector<float> *getAllAdvance(Utf8String &line, int y);
  ColorKind classifyColorKind(FT_Face face);
  void insertBlankEntry(char32_t c);
};

#endif
