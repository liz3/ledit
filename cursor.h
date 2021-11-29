#ifndef CURSOR_H
#define CURSOR_H

//#include <math>
#include <string>
#include <map>
#include <vector>
#include <sstream>
#include <fstream>
struct PosEntry {
  int x,y, skip;
};
class Cursor {
 public:
  std::vector<std::string> lines;
  std::map<std::string, PosEntry> saveLocs;
  int x = 0;
  int y = 0;
  int xSave = 0;
  int skip = 0;
  float height = 0;
  float lineHeight = 0;
  int maxLines = 0;
  std::string* bind = nullptr;
  void setBounds(float height, float lineHeight) {
    this->height = height;
    this->lineHeight = lineHeight;
    maxLines = std::floor(height / lineHeight) -1;
  }
  void bindTo(std::string* entry) {
    bind = entry;
    xSave = x;
    x = entry->length();
  }
  void unbind() {
    bind = nullptr;
    x = xSave;
  }
  std::string search(std::string what) {
    int i = 0;
    for(auto line : lines) {
      auto where = line.find(what);
      if(where != std::string::npos) {
        y = i;
        x = where;
        xSave = where;
        center(i);
        return "At: " + std::to_string(y + 1) + ":" + std::to_string(where + 1);
      }
      i++;
    }
    return "Not found";
  }
  void gotoLine(int l) {
    if(l-1 > lines.size())
      return;
    x = 0;
    xSave = 0;
    y = l-1;
    center(l);
  }
  void center(int l) {
    if(l < maxLines /2 || lines.size() < l)
      skip = 0;
    else
      skip = l - (maxLines / 2);


  }
  std::vector<std::string> split(std::string base, std::string delimiter) {
    std::vector<std::string> final;
    size_t pos = 0;
    std::string token;
    while ((pos = base.find(delimiter)) != std::string::npos) {
      token = base.substr(0, pos);
      final.push_back(token);
      base.erase(0, pos + delimiter.length());
    }
    final.push_back(base);
    return final;
  }

  Cursor() {
    lines.push_back("");
  }
  // Cursor(std::vector<std::string> contents) {
  //   lines = contents;

   Cursor(std::string path) {
     std::stringstream ss;
     std::ifstream stream(path);
     ss << stream.rdbuf();
     lines = split(ss.str(), "\n");
     stream.close();

  }
  bool openFile(std::string oldPath, std::string path) {
    std::ifstream stream(path);
    if(!stream.is_open()) {
      return false;
    }
    if(oldPath.length()) {
      PosEntry entry;
      entry.x = xSave;
      entry.y = y;
      entry.skip = skip;
      saveLocs[oldPath] = entry;
    }
    if(saveLocs.count(path)) {
        PosEntry savedEntry = saveLocs[path];
        x = savedEntry.x;
        y = savedEntry.y;
        skip = savedEntry.skip;
    } else {
      x=0;
      y=0;
      skip = 0;
   }
    xSave = x;
    std::stringstream ss;
    ss << stream.rdbuf();
    lines = split(ss.str(), "\n");
    if(skip > lines.size() - maxLines)
      skip = 0;
    stream.close();
    return true;
  }
  void append(char c) {
    if(c == '\n' && bind == nullptr) {
      auto pos = lines.begin() + y;
      std::string* current = &lines[y];
      bool isEnd = x == current->length();
      if(isEnd) {
        lines.insert(pos+1,"");
      } else {
        if(x== 0) {
          lines.insert(pos, "");
        } else {
          std::string toWrite = current->substr(0, x);
          std::string next = current->substr(x);
          lines[y] = toWrite;
          lines.insert(pos+1, next);
        }

      }
      y++;
      x = 0;
    }else {
      auto* target = bind ? bind :  &lines[y];
      std::string content;
      content += c;
      target->insert(x, content);
      x++;
    }
  }
  void append(std::string content) {
    auto* target = bind ? bind : &lines[y];
    target->insert(x, content);
    x += content.length();
  }

  std::string getCurrentAdvance() {
    if(bind)
      return bind->substr(0, x);
    return lines[y].substr(0, x);
  }
  void removeOne() {
    std::string* target = bind ? bind :  &lines[y];
    if(x == 0) {
      if(y == 0 || bind)
        return;
        std::string* copyTarget = &lines[y-1];
        int xTarget = copyTarget->length();
        if (target->length() > 0) {
          copyTarget->append(*target);
        }
        lines.erase (lines.begin()+y);
        y--;
        x = xTarget;

    } else {
      target->erase(x-1, 1);
      x--;
    }
  }
  void moveUp() {
    if(y == 0 || bind)
      return;
   std::string* target = &lines[y-1];
   int targetX = target->length() < x ? target->length() : x;
   x = targetX;
   y--;
  }
  void moveDown() {
    if(bind || y == lines.size()-1)
      return;
   std::string* target = &lines[y+1];
   int targetX = target->length() < x ? target->length() : x;
   x = targetX;
   y++;
  }

  void jumpStart() {
    x = 0;
  }

  void jumpEnd() {
    if(bind)
      x = bind->length();
    else
      x = lines[y].length();
  }


  void moveRight() {
    std::string* current = bind ? bind : &lines[y];
    if(x == current->length()) {
      if(y == lines.size()-1 || bind)
        return;
      y++;
      x = 0;
    } else {
      x++;
    }
  }
  void moveLeft() {
    std::string* current = bind ? bind :  &lines[y];
    if(x == 0) {
      if(y == 0 || bind)
        return;
      std::string* target = & lines[y-1];
      y--;
      x = target->length();
    } else {
      x--;
    }
  }
  void saveTo(std::string path) {
    std::ofstream stream(path,  std::ofstream::out);
    if(!stream.is_open()) {
      std::cout << "stream not open" << "\n";
      return;
    }
    for(size_t i = 0; i < lines.size(); i++) {
      stream << lines[i];
      if(i < lines.size() -1)
        stream << "\n";
    }
    stream.flush();
    stream.close();
  }
  std::string getContent() {

    int end = skip + maxLines;
    if(end >= lines.size()) {
      end = lines.size();
      skip = end-maxLines;
      if(skip < 0)
        skip = 0;
    }
    if(y == end && end < (lines.size())) {

      skip++;
      end++;

    } else if(y < skip && skip > 0) {
      skip--;
      end--;
    }
    std::stringstream ss;
    for(size_t i = skip; i < end; i++) {
      ss << lines[i];
      if(i < end -1)
        ss << "\n";
    }
    return ss.str();
  }

};
#endif
