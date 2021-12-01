#ifndef HIGHLIGHTING_H
#define HIGHLIGHTING_H
#include "la.h"
#include <string>
#include <map>
#include <sstream>
struct Language {
  std::string modeName;
  std::vector<std::string> keyWords;
  std::vector<std::string> specialWords;
  std::string singleLineComment;
  std::pair<std::string, std::string> multiLineComment;
  std::string stringCharacters;
  char escapeChar;
  Vec4f string_color;
  Vec4f default_color;
  Vec4f keyword_color;
  Vec4f special_color;
  Vec4f comment_color;
  std::vector<std::string> fileExtensions;

};
struct HighlighterState {
  int start;
  bool busy;
  bool wasReset;
  int mode;
  std::string buffer;
  char stringChar;
};
class Highlighter {
public:
  std::string languageName;
  Language language;
  const std::string whitespace = " \t\n[]{}();.,";
  bool isNonChar(char c) {
    return whitespace.find(c) != std::string::npos;
  }
  std::string cachedContent = "";
  std::map<int, Vec4f> cached;
  std::map<int, std::pair<int, int>> lineIndex;
  bool wasCached = false;
  void setLanguage(Language lang, std::string name) {
    languageName = name;
    wasCached = false;
    language = lang;
  }
  std::map<int, Vec4f>* highlight(std::vector<std::string>& lines) {
    std::stringstream stream;
    for(size_t i = 0; i < lines.size(); i++) {
      stream << lines[i];
      if(i < lines.size() -1)
        stream << "\n";
    }
    return highlight(stream.str());
  }
  std::map<int, Vec4f>* get() {
    return &cached;
  }
  std::map<int, Vec4f>* highlight(std::string raw) {
    if(wasCached && raw == cachedContent)
      return &cached;
    HighlighterState state {0, false, false, 0, "", 0};
    int startIndex = 0;
    int y = 0;
    lineIndex.clear();
    Vec4f string_color = language.string_color;
    Vec4f default_color = language.default_color;
    Vec4f keyword_color = language.keyword_color;
    Vec4f special_color = language.special_color;
    Vec4f comment_color = language.comment_color;
    char last = 0;
    std::map<int, Vec4f> entries;
    size_t i;
    for(i = 0; i < raw.length(); i++) {
      char current = raw[i];
      if(current == '\n') {
        int endIndex = entries.size();
        lineIndex[y++] = std::pair<int, int>(startIndex, endIndex);
        startIndex = endIndex;

      }
      if(language.stringCharacters.find(current) != std::string::npos && (last != language.escapeChar || (last == language.escapeChar && i >1 && raw[i-2] == language.escapeChar))) {
        if(state.mode == 0 && !state.busy) {
          state.mode = 1;
          state.busy = true;
          state.start = i;
          state.stringChar = current;
          entries[i] = string_color;
        } else if (state.mode == 1 && current == state.stringChar) {
          state.busy = false;
          state.mode = 0;
          state.start = 0;
          entries[i+1] = default_color;
        }
      } else if (state.busy && state.mode == 3 && hasEnding(state.buffer, language.multiLineComment.second)) {
        state.mode = 0;
        state.busy = false;
        entries[i] = default_color;
      } else if (state.busy && state.mode == 2 && current == '\n') {
        state.buffer = "";
        state.busy = false;
        state.mode = 0;
        entries[i] = default_color;
      } else if (hasEnding(state.buffer+current, language.singleLineComment) && !state.busy) {

        entries[i  - (language.singleLineComment.length()-1)] = comment_color;
        state.busy = true;
        state.mode = 2;
        state.buffer = "";
      }else if (hasEnding(state.buffer, language.multiLineComment.first) && !state.busy) {
        entries[i- language.multiLineComment.first.length()] = comment_color;
        state.buffer = "";
        state.busy = true;
        state.mode = 3;
      } else if(isNonChar(current) && !state.busy && state.buffer.length() && !state.wasReset) {

        if (std::find(language.keyWords.begin(), language.keyWords.end(), state.buffer)!= language.keyWords.end()) {

          entries[state.start] = keyword_color;
          entries[i] = default_color;
          state.wasReset = true;
          state.buffer = "";
        } else if (std::find(language.specialWords.begin(), language.specialWords.end(), state.buffer)!= language.specialWords.end()) {
          entries[state.start] = special_color;
          entries[i] = default_color;
          state.wasReset = true;
          state.buffer = "";
        }

      } else if(!state.busy && isNonChar(last) && !isNonChar(current)) {
        state.wasReset = false;
        state.buffer = "";
        state.start = i;
      }

      state.buffer += current;
      last = current;

    }

    //        std::cout << "lol: " << state.buffer << "end\n";
    if(state.buffer.length()) {

      if (std::find(language.keyWords.begin(), language.keyWords.end(), state.buffer)!= language.keyWords.end() && nextIsValid(raw, i)) {
        entries[state.start] = keyword_color;
        entries[i] = default_color;
        state.wasReset = true;
      } else if (std::find(language.specialWords.begin(), language.specialWords.end(), state.buffer)!= language.specialWords.end() && nextIsValid(raw, i)) {
        entries[state.start] = special_color;
        entries[i] = default_color;
        state.wasReset = true;
      }  else if (hasEnding(state.buffer, language.singleLineComment)) {

        entries[offset(i) - language.singleLineComment.length()] = comment_color;
        state.busy = true;
        state.mode = 2;
        state.buffer = "";
      }else if (hasEnding(state.buffer, language.multiLineComment.first)) {
        entries[i] = comment_color;
        state.buffer = "";
        state.busy = true;
        state.mode = 3;
      }
    }
    int endIndex = entries.size();
    lineIndex[y++] = std::pair<int, int>(startIndex, endIndex);
    cached = entries;
    cachedContent = raw;
    wasCached = true;
    return &cached;
  }
private:
  bool nextIsValid(std::string str, int i) {
    return i >= str.length()-1 || isNonChar(str[i+1]);
  }
  int offset(int i) {
    return i+1;
  }
  bool hasEnding (std::string const &fullString, std::string const &ending) {
    if (fullString.length() >= ending.length()) {
      return (0 == fullString.compare (fullString.length() - ending.length(), ending.length(), ending));
    } else {
      return false;
    }
  }
};

#endif
