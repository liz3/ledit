#ifndef UTILS_H
#define UTILS_H
#include "utf8String.h"
const Utf8String wordSeperator = U" \t\n[]{}/\\*()=_-+,.\"";
const Utf8String wordSeperator2 = U" \t";
const std::vector<std::pair<char32_t, char32_t>> PAIRS = {
     {'{', '}'}, {'(', ')'}, {'[', ']'}};
std::string file_to_string(std::string path);
std::string toFixed(double number, int precision);
bool hasEnding(Utf8String fullString, Utf8String ending);
bool hasEnding(std::string fullString, std::string ending);
bool isSafeNumber(std::string value);
bool string_to_file(std::string path, std::string content);
#endif
