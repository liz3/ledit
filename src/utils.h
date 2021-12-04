#ifndef UTILS_H
#define UTILS_H

std::string file_to_string(std::string path) {
  std::ifstream stream(path);
  std::stringstream ss;
  ss << stream.rdbuf();
  stream.close();
  return ss.str();
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
