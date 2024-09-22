#ifndef SHADER_H
#define SHADER_H
#include "la.h"
#include <vector>
#include "glad.h"



struct CharacterEntry {
  float width;
  float height;
  float top;
  float left;
  float advance;
  float advanceY;
  float offset;
  uint8_t *data = nullptr;
  int xPos;
  char32_t c;
  bool hasColor;
  CharacterEntry();
  ~CharacterEntry();
  CharacterEntry(const CharacterEntry &other);

  CharacterEntry &operator=(const CharacterEntry &other);
};
struct RenderChar {
  Vec2f pos;
  Vec2f size;
  Vec2f uv_pos;
  Vec2f uv_size;
  Vec4f fg_color;
  Vec4f bg_color;
  float hasColor;
};
struct SelectionEntry {
  Vec2f pos;
  Vec2f size;
};
class Shader {
public:
  GLuint pid;
  std::vector<GLuint> shader_ids;
  Shader(std::string vertex, std::string fragment,
         std::vector<std::string> others);
  void set2f(std::string name, float x, float y);
  void set4f(std::string name, float x, float y, float z, float w);
   void set4f(std::string name, Vec4f in);
  void set1f(std::string name, float v);
  void use();
  void removeShader();

private:
  GLuint compileSimple(GLuint type, std::string path);
  void checkCompileErrors(GLuint shader, std::string type);
  const std::string shaderTypeString(GLuint shader);
};

#endif
