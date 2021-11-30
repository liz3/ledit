#ifndef SHADER_CONSTANT_H
#define SHADER_CONSTANT_H

#include <string>

const std::string text_shader_vert = R"(
#version 330 core


layout(location = 0) in vec2 pos;
layout(location = 1) in vec2 size;
layout(location = 2) in vec2 uv_pos;
layout(location = 3) in vec2 uv_size;
layout(location = 4) in vec4 fg_color;
layout(location = 5) in vec4 bg_color;

out vec2 uv;
out vec2 glyph_uv_pos;
out vec2 glyph_uv_size;
out vec4 glyph_fg_color;
out vec4 glyph_bg_color;
uniform vec2 resolution;
vec2 camera_project(vec2 point) {
return 2* (point) * (1 / resolution);
}

void main() {
    uv = vec2(float(gl_VertexID & 1), float((gl_VertexID >> 1) & 1));
    gl_Position = vec4(camera_project(uv * (size) + (pos)), 0.0, 1.0);
    glyph_uv_pos = uv_pos;
    glyph_uv_size = uv_size;
    glyph_fg_color = fg_color;
    glyph_bg_color = vec4(0.0);
}

)";

const std::string text_shader_frag = R"(
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
)";

const std::string camera_shader_vert = R"(#version 330 core
uniform vec2 resolution;
 float camera_scale;
 vec2 camera_pos;

vec2 camera_project(vec2 point)
{

  camera_scale = 1.0;
  camera_pos = vec2(0);
  return 2 * (point - camera_pos) * camera_scale / resolution;
}
)";

const std::string cursor_shader_vert = R"(
#version 330 core

uniform vec2 resolution;
uniform vec2 cursor_pos;

#define WIDTH 4.0
uniform float cursor_height;

out vec2 uv;

vec2 camera_project(vec2 point);

void main() {
    uv = vec2(float(gl_VertexID & 1),
              float((gl_VertexID >> 1) & 1));
    gl_Position = vec4(
        camera_project(uv * vec2(WIDTH, cursor_height) + cursor_pos),
        0.0,
        1.0);
}

)";

const std::string cursor_shader_frag = R"(


#version 330 core

#define PERIOD 0.8
#define BLINK_THRESHOLD 0.5

uniform float time;
uniform float last_stroke;
out vec4 diffuseColor;
void main() {
    float t = time - last_stroke;
    float threshold = float(t < BLINK_THRESHOLD);
    float blink = mod(floor(t / PERIOD), 2);
    diffuseColor = vec4(0.8,0.8,1.0,1.0) *  min(threshold + blink, 1.0);
}

)";

#endif
