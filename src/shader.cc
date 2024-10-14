#include "shader.h"
#include <iostream>
#include <cstring>

CharacterEntry::CharacterEntry() {
  
}

CharacterEntry::~CharacterEntry() {
  if (data != nullptr) {
    delete[] data;
    data = nullptr;
  }
}
CharacterEntry::CharacterEntry(const CharacterEntry &other) { *this = other; }
CharacterEntry &CharacterEntry::operator=(const CharacterEntry &other) {
  width = other.width;
  height = other.height;
  top = other.top;
  left = other.left;
  advance = other.advance;
  advanceY = other.advanceY;
  offset = other.offset;
  xPos = other.xPos;
  c = other.c;
  hasColor = other.hasColor;
  if (other.data != nullptr && data == nullptr) {
    this->data = new uint8_t[(int)width * (int)height * 4];
    memcpy(data, other.data, (int)width * (int)height * 4);
  }
  return *this;
}
Shader::Shader(std::string vertex, std::string fragment,
               std::vector<std::string> others) {
  auto vertex_shader = compileSimple(GL_VERTEX_SHADER, vertex);
  auto fragment_shader = compileSimple(GL_FRAGMENT_SHADER, fragment);
  pid = glCreateProgram();
  glAttachShader(pid, vertex_shader);
  glAttachShader(pid, fragment_shader);
  shader_ids.push_back(vertex_shader);
  shader_ids.push_back(fragment_shader);
  for (auto &other : others) {
    auto shader_id = compileSimple(GL_VERTEX_SHADER, other);
    glAttachShader(pid, shader_id);
    shader_ids.push_back(shader_id);
  }
  glLinkProgram(pid);
  checkCompileErrors(pid, "PROGRAM");
}
void Shader::set2f(std::string name, float x, float y) {
  glUniform2f(glGetUniformLocation(pid, name.c_str()), x, y);
}
void Shader::set4f(std::string name, float x, float y, float z, float w) {
  glUniform4f(glGetUniformLocation(pid, name.c_str()), x, y, z, w);
}
void Shader::set4f(std::string name, Vec4f in) {
  glUniform4f(glGetUniformLocation(pid, name.c_str()), in.x, in.y, in.z, in.w);
}
void Shader::set1f(std::string name, float v) {
  glUniform1f(glGetUniformLocation(pid, name.c_str()), v);
}
void Shader::use() { glUseProgram(pid); }
void Shader::removeShader() { glDeleteProgram(pid); }
GLuint Shader::compileSimple(GLuint type, std::string path) {
  std::string content = path;
  auto id = glCreateShader(type);
  const char *contentp = content.c_str();
  glShaderSource(id, 1, &contentp, nullptr);
  glCompileShader(id);
  checkCompileErrors(id, shaderTypeString(type));
  return id;
}
void Shader::checkCompileErrors(GLuint shader, std::string type) {
  GLint success;
  GLchar infoLog[1024];
  if (type != "PROGRAM") {
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
      glGetShaderInfoLog(shader, 1024, NULL, infoLog);
      std::cout
          << "ERROR::SHADER_COMPILATION_ERROR of type: " << type << "\n"
          << infoLog
          << "\n -- --------------------------------------------------- -- "
          << std::endl;
    }
  } else {
    glGetProgramiv(shader, GL_LINK_STATUS, &success);
    if (!success) {
      glGetProgramInfoLog(shader, 1024, NULL, infoLog);
      std::cout
          << "ERROR::PROGRAM_LINKING_ERROR of type: " << type << "\n"
          << infoLog
          << "\n -- --------------------------------------------------- -- "
          << std::endl;
    }
  }
}
const std::string Shader::shaderTypeString(GLuint shader) {
  switch (shader) {
  case GL_VERTEX_SHADER:
    return "GL_VERTEX_SHADER";
  case GL_FRAGMENT_SHADER:
    return "GL_FRAGMENT_SHADER";
  default:
    return "(Unknown)";
  }
}
