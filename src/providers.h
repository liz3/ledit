#ifndef PROVIDERS_H
#define PROVIDERS_H

#include <vector>
#include <filesystem>
namespace fs = std::filesystem;

class Provider {
public:
  std::string lastProvidedFolder;
  std::vector<std::string> folderEntries;
  int offset = 0;
  std::string getLast() {
   if(!folderEntries.size())
      return "";
    return folderEntries[offset];
  }
std::string getFileToOpen(std::string path) {
  if(!fs::exists(path) || !fs::is_directory(path))
    return "";
  if(lastProvidedFolder == path) {
    offset++;
    if(offset == folderEntries.size())
      offset = 0;
    return folderEntries[offset];
  }
  folderEntries.clear();
  for(auto const & dir_entry : fs::directory_iterator{path}) {
    folderEntries.push_back(dir_entry.path().string());
  }
  offset = 0;
  lastProvidedFolder = path;
  return folderEntries[offset];
 }
};

#endif