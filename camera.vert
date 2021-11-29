#version 330 core
uniform vec2 resolution;
 float camera_scale;
 vec2 camera_pos;

vec2 camera_project(vec2 point)
{

  camera_scale = 1.0;
  camera_pos = vec2(0);
  return  (point - camera_pos) * camera_scale / resolution;
}
