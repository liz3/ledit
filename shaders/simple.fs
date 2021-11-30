#version 330 core

uniform sampler2D font;

in vec2 uv;
in vec2 glyph_uv_pos;
in vec2 glyph_uv_size;
in vec4 glyph_fg_color;
in vec4 glyph_bg_color;

out vec4 color;
void main() {
     vec2 t = glyph_uv_pos + glyph_uv_size * uv;
     vec4 sampled = vec4(1.0, 1.0, 1.0, texture(font, t).x);
     color = glyph_fg_color * sampled;
}
