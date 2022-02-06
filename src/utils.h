#ifndef UTILS_H
#define UTILS_H
const std::u16string wordSeperator = u" \t\n[]{}/\\*()=_-,.";
std::string file_to_string(std::string path) {
  std::ifstream stream(path);
  std::stringstream ss;
  ss << stream.rdbuf();
  stream.close();
  return ss.str();
}
bool hasEnding (std::u16string fullString, std::u16string ending) {
    if (fullString.length() >= ending.length()) {
        return (0 == fullString.compare (fullString.length() - ending.length(), ending.length(), ending));
    } else {
        return false;
    }
}
bool hasEnding (std::string fullString, std::string ending) {
    if (fullString.length() >= ending.length()) {
        return (0 == fullString.compare (fullString.length() - ending.length(), ending.length(), ending));
    } else {
        return false;
    }
}
bool isSafeNumber(std::string value) {
  for(const char& c : value) {
    if(c < '0' || c > '9')
      return false;
  }
  return true;
}
bool string_to_file(std::string path, std::string content) {
  std::ofstream stream(path);
  if(!stream.is_open())
    return false;
  stream << content;
  stream.close();
  return true;
}
#endif
