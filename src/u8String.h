#ifndef U8STRING_H
#define U8STRING_H

#include <string>
#include <codecvt>
#ifdef __linux__
#include <locale>
#endif
#include "utf8String.h"
typedef union char_s {
  char16_t value;
  uint8_t arr[2];
} char_t;

static std::string convert_str(Utf8String in) {
  return in.getStr();
}

static Utf8String create(std::string u) {
  return Utf8String(u);
}
inline Utf8String numberToString(int value) {
  std::string val = std::to_string(value);
  return create(val);
}

#endif
