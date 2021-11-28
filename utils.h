#ifndef GFLW_LIZ_UTILS_H
#define GFLW_LIZ_UTILS_H

struct Glyph_Metric {
  float ax; // advance.x
  float ay; // advance.y

  float bw; // bitmap.width;
  float bh; // bitmap.rows;

  float bl; // bitmap_left;
  float bt; // bitmap_top;

  float tx; // x offset of glyph in texture coordinates
};
struct Free_Glyph {
    Vec2f pos;
    Vec2f size;
    Vec2f uv_pos;
    Vec2f uv_size;
    Vec4f fg_color;
    Vec4f bg_color;
};
enum Free_Glyph_Attr {
    FREE_GLYPH_ATTR_POS = 0,
    FREE_GLYPH_ATTR_SIZE,
    FREE_GLYPH_ATTR_UV_POS,
    FREE_GLYPH_ATTR_UV_SIZE,
    FREE_GLYPH_ATTR_FG_COLOR,
    FREE_GLYPH_ATTR_BG_COLOR,
    COUNT_FREE_GLYPH_ATTRS,
};
 struct Attr_Def {
    size_t offset;
    GLint comps;
    GLenum type;
};

static const Attr_Def glyph_attr_defs[COUNT_FREE_GLYPH_ATTRS] = {
    [FREE_GLYPH_ATTR_POS]   = {
        .offset = offsetof(Free_Glyph, pos),
        .comps = 2,
        .type = GL_FLOAT
    },
    [FREE_GLYPH_ATTR_SIZE]   = {
        .offset = offsetof(Free_Glyph, size),
        .comps = 2,
        .type = GL_FLOAT
    },
    [FREE_GLYPH_ATTR_UV_POS]    = {
        .offset = offsetof(Free_Glyph, uv_pos),
        .comps = 2,
        .type = GL_FLOAT
    },
    [FREE_GLYPH_ATTR_UV_SIZE]    = {
        .offset = offsetof(Free_Glyph, uv_size),
        .comps = 2,
        .type = GL_FLOAT
    },
    [FREE_GLYPH_ATTR_FG_COLOR] = {
        .offset = offsetof(Free_Glyph, fg_color),
        .comps = 4,
        .type = GL_FLOAT
    },
    [FREE_GLYPH_ATTR_BG_COLOR] = {
        .offset = offsetof(Free_Glyph, bg_color),
        .comps = 4,
        .type = GL_FLOAT
    },
};
class Utils {
 public:

 private:
};

#endif
