#ifndef HIGHLIGHTING_H
#define HIGHLIGHTING_H
#include "la.h"
#include "providers.h"
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
  std::vector<std::string> fileExtensions;
};
struct LanguageExpanded {
  std::u16string modeName;
  std::vector<std::u16string> keyWords;
  std::vector<std::u16string> specialWords;
  std::u16string singleLineComment;
  std::pair<std::u16string, std::u16string> multiLineComment;
  std::u16string stringCharacters;
  char16_t escapeChar;
};
struct HighlighterState {
  int start;
  bool busy;
  bool wasReset;
  int mode;
  std::u16string buffer;
  char stringChar;
};
class Highlighter {
public:
  std::u16string languageName;

  LanguageExpanded language;
  const std::u16string whitespace = u" \t\n[]{}();.,";
  bool isNonChar(char16_t c) {
    return whitespace.find(c) != std::string::npos;
  }
  std::u16string cachedContent = u"";
  std::map<int, Vec4f> cached;
  std::map<int, std::pair<int, int>> lineIndex;
  Vec4f lastEntry;
  bool wasCached = false;
  void setLanguage(Language lang, std::string name) {
    language.modeName = create(lang.modeName);
    language.keyWords.clear();
    for(auto& entry : lang.keyWords) {
      language.keyWords.push_back(create(entry));
    }
    language.specialWords.clear();
    for(auto& entry : lang.specialWords) {
      language.specialWords.push_back(create(entry));
    }
    if(lang.singleLineComment.length()) {
      language.singleLineComment = create(lang.singleLineComment);
    } else {
      language.singleLineComment = u"";
    }
    if(lang.multiLineComment.first.length()) {
      language.multiLineComment = std::pair(create(lang.multiLineComment.first), create(lang.multiLineComment.second));
    } else {
      language.multiLineComment = std::pair(u"", u"");
    }
    language.stringCharacters = create(lang.stringCharacters);
    language.escapeChar = (char16_t) lang.escapeChar;

    languageName = create(name);
    wasCached = false;
  }
  std::map<int, Vec4f>* highlight(std::vector<std::u16string>& lines, EditorColors* colors, int skip) {
    std::u16string str;
    for(size_t i = 0; i < lines.size(); i++) {
      str += lines[i];
      if(i < lines.size() -1)
        str += u"\n";
    }
    return highlight(str, colors, skip);
  }
  std::map<int, Vec4f>* get() {
    return &cached;
  }
  std::map<int, Vec4f>* highlight(std::u16string raw, EditorColors* colors, int skip) {
    if(wasCached && raw == cachedContent)
      return &cached;
    HighlighterState state {0, false, false, 0, u"", 0};
    int startIndex = 0;
    int y = 0;
    lineIndex.clear();
    Vec4f string_color = colors->string_color;
    Vec4f default_color = colors->default_color;
    Vec4f keyword_color = colors->keyword_color;
    Vec4f special_color = colors->special_color;
    Vec4f comment_color = colors->comment_color;
    char16_t last = 0;
    std::map<int, Vec4f> entries;
    size_t i;
    size_t lCount = 0;
    bool wasTrigered = false;
    for(i = 0; i < raw.length(); i++) {
      char16_t current = raw[i];
      if(current == '\n') {
        int endIndex = entries.size();
        lineIndex[y++] = std::pair<int, int>(startIndex, endIndex);
        startIndex = endIndex;
        if(lCount++ > skip && wasCached)
          break;
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
        state.buffer = u"";
        state.busy = false;
        state.mode = 0;
        entries[i] = default_color;
      } else if (language.singleLineComment.length() && hasEnding(state.buffer+current, language.singleLineComment) && !state.busy) {

        entries[i  - (language.singleLineComment.length()-1)] = comment_color;
        state.busy = true;
        state.mode = 2;
        state.buffer = u"";
      } else if (language.multiLineComment.first.length() && hasEnding(state.buffer, language.multiLineComment.first) && !state.busy) {
        entries[i- language.multiLineComment.first.length()] = comment_color;
        state.buffer = u"";
        state.busy = true;
        state.mode = 3;
      } else if(isNonChar(current) && !state.busy && state.buffer.length() && !state.wasReset) {

        if (std::find(language.keyWords.begin(), language.keyWords.end(), state.buffer)!= language.keyWords.end()) {

          entries[state.start] = keyword_color;
          entries[i] = default_color;
          state.wasReset = true;
          state.buffer = u"";
        } else if (std::find(language.specialWords.begin(), language.specialWords.end(), state.buffer)!= language.specialWords.end()) {
          entries[state.start] = special_color;
          entries[i] = default_color;
          state.wasReset = true;
          state.buffer = u"";
        }

      } else if(!state.busy && isNonChar(last) && !isNonChar(current)) {
        state.wasReset = false;
        state.buffer = u"";
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
        state.buffer = u"";
      }else if (hasEnding(state.buffer, language.multiLineComment.first)) {
        entries[i] = comment_color;
        state.buffer = u"";
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
  bool nextIsValid(std::u16string str, int i) {
    return i >= str.length()-1 || isNonChar(str[i+1]);
  }
  int offset(int i) {
    return i+1;
  }
  bool hasEnding (std::u16string const &fullString, std::u16string const &ending) {
    if (fullString.length() >= ending.length()) {
      return (0 == fullString.compare (fullString.length() - ending.length(), ending.length(), ending));
    } else {
      return false;
    }
  }
};

#endif
