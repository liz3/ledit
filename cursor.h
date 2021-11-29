#ifndef CURSOR_H
#define CURSOR_H

//#include <math>
#include <string>
#include <vector>
#include <sstream>
#include <fstream>
class Cursor {
 public:
  std::vector<std::string> lines;
  int x = 0;
  int y = 0;
  int skip = 0;
  float height = 0;
  float lineHeight = 0;;

  void setBounds(float height, float lineHeight) {
    this->height = height;
    this->lineHeight = lineHeight;
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
  void append(char c) {
    if(c == '\n') {
      auto pos = lines.begin() + y;
      std::string* current = &lines[y];
      bool isEnd = x == current->length();
      lines.insert(isEnd ? pos+1: pos, "");
      y++;
      x = 0;
    }else {
      auto* target = &lines[y];
      std::string content;
      content += c;
      target->insert(x, content);
      x++;
    }
  }
  std::string getCurrentAdvance() {
    return lines[y].substr(0, x);
  }
  void removeOne() {
    std::string* target = &lines[y];
    if(x == 0) {
      if(y == 0)
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
    if(y == 0)
      return;
   std::string* target = &lines[y-1];
   int targetX = target->length() < x ? target->length() : x;
   x = targetX;
   y--;
  }
  void moveDown() {
    if(y == lines.size()-1)
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
    x = lines[y].length();
  }


  void moveRight() {
    std::string* current = &lines[y];
    if(x == current->length()) {
      if(y == lines.size()-1)
        return;
      y++;
      x = 0;
    } else {
      x++;
    }
  }

  void moveLeft() {
    std::string* current = &lines[y];
    if(x == 0) {
      if(y == 0)
        return;
      std::string* target = & lines[y-1];
      y--;
      x = target->length();
    } else {
      x--;
    }
  }

  std::string getContent() {
    int maxLines = std::floor(height / lineHeight)-1;
    int end = skip + maxLines;
    if(end >= lines.size()) {
      end = lines.size();
      skip = end-maxLines;
    }
    if(y == end && end < (lines.size())) {

      skip++;
      end++;

    } else if(y == skip && skip > 0) {
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
