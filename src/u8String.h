#ifndef U8STRING_H
#define U8STRING_H

#include <string>
#include <codecvt>
typedef union char_s {
  char16_t value;
  uint8_t arr[2];
} char_t;

  std::wstring_convert<std::codecvt_utf8_utf16<char16_t>,char16_t> utfConverter;
static std::string convert_str(std::u16string data) {
  std::string b = utfConverter.to_bytes(data);
  return b;
}

static std::u16string create(std::string container) {
  std::u16string b = utfConverter.from_bytes(container);
  return b;
}
std::u16string numberToString(int value) {
  std::string val = std::to_string(value);
  return create(val);
}

#endif
