#ifndef LANGUAGES_H
#define LANGUAGES_H

#include "highlighting.h"
#include "la.h"
const Language CPP = {
  "C++",
  {"for", "while", "if", "int", "float", "return", "double", "true", "false", "else", "void", "delete", "struct", "class"},
  {"#include", "std::", "#ifdef", "#define", "#endif"},
  "//",
  {"/*", "*/"},
  "\"'",
  '\\',
  vec4f(0.2, 0.6, 0.4, 1.0),
  vec4fs(0.95),
  vec4f(0.6, 0.1, 0.2, 1.0),
  vec4f(0.2, 0.2, 0.8, 1.0),
  vec4fs(0.5),
  {"cc", "h", "cpp", "hpp"}
};
const Language* has_language(std::string ext) {
  if(std::find(CPP.fileExtensions.begin(), CPP.fileExtensions.end(), ext)!=  CPP.fileExtensions.end())
    return &CPP;
  return nullptr;
}
#endif
