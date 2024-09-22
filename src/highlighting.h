#ifndef HIGHLIGHTING_H
#define HIGHLIGHTING_H
#include "la.h"

#include <string>
#include <map>
#include <sstream>
#include <unordered_map>
#include <vector>
#include "utf8String.h"

const std::string DEFAULT_WHITESPACE_CHARS = " \t\n[]{}();:.,*-+/";
struct EditorColors {
  Vec4f string_color = vec4f(0.2, 0.6, 0.4, 1.0);
  Vec4f default_color = vec4fs(0.95);
  Vec4f keyword_color = vec4f(0.6, 0.1, 0.2, 1.0);
  Vec4f special_color = vec4f(0.2, 0.2, 0.8, 1.0);
  Vec4f number_color = vec4f(0.2, 0.2, 0.6, 1.0);
  Vec4f symbol_color = vec4fs(0.67);
  Vec4f comment_color = vec4fs(0.5);
  Vec4f fold_color = vec4fs(0.5);
  Vec4f background_color = vec4f(0, 0, 0, 1.0);
  Vec4f highlight_color = vec4f(0.1, 0.1, 0.1, 1.0);
  Vec4f selection_color = vec4f(0.7, 0.7, 0.7, 0.6);
  Vec4f status_color = vec4f(0.8, 0.8, 1.0, 0.9);
  Vec4f minibuffer_color = vec4fs(1.0);
  Vec4f line_number_color = vec4fs(0.8);
  Vec4f cursor_color_standard = vec4f(0.8, 0.8, 0.8, 1);
  Vec4f cursor_color_vim = vec4f(0.8, 0.8, 0.8, 0.5);
};
struct Language {
  std::string modeName;
  std::vector<std::string> keyWords;
  std::vector<std::string> specialWords;
  std::string singleLineComment;
  std::pair<std::string, std::string> multiLineComment;
  std::string stringCharacters;
  char escapeChar;
  std::vector<std::string> fileExtensions;
  std::string whitespace = DEFAULT_WHITESPACE_CHARS;
  bool keywords_case_sensitive = true;
  bool special_case_sensitive = true;
  bool tabIndent = false;
  std::string symbols = "+-/*()[]{};:=<>.,:";
  std::string indentStr = "{";
  std::string outdentStr = "}";
};
struct LanguageExpanded {
  Utf8String modeName;
  std::unordered_map<std::string, bool> keyWords;
  std::unordered_map<std::string, bool> specialWords;
  Utf8String singleLineComment;
  std::pair<Utf8String, Utf8String> multiLineComment;
  Utf8String stringCharacters;
  std::unordered_map<char32_t, bool> whitespace;
  char32_t escapeChar;
  bool keywords_case_sensitive = true;
  bool special_case_sensitive = true;
  bool tabIndent = false;
  Utf8String indentStr;
  Utf8String outdentStr;
  Utf8String symbols;
};
struct HighlighterState {
  int start;
  bool busy;
  bool wasReset;
  int mode;
  Utf8String buffer;
  char stringChar;
  bool lastOperator;
};
struct SavedState {
  HighlighterState state;
  Vec4f color;
  bool loaded = false;
};
class Highlighter {
public:
  Utf8String languageName;
  LanguageExpanded language;
  std::map<int, Vec4f> cached;
  std::map<int, std::pair<int, int>> lineIndex;
  std::map<int, int> indentLevels;
  Vec4f lastEntry;
  int lastSkip = 0;
  int lastMax = 0;
  size_t last_history_size = 0;
  int lastY = 0;
  SavedState savedState;
  bool wasCached = false;
  bool wasEntire = false;
  bool isNonChar(char32_t c);
  bool isNumber(char32_t c);
  bool isNumberEnd(char32_t c, bool hexa);
  std::string lowerCase(const std::string &str);
  void setLanguage(Language lang, std::string name);
  std::map<int, Vec4f> *highlight(std::vector<Utf8String> &lines,
                                  EditorColors *colors, int skip, int maxLines,
                                  int y, size_t history_size);
  std::map<int, Vec4f> *get();
  std::map<int, Vec4f> *highlight(Utf8String &raw, EditorColors *colors,
                                  int skip, int maxLines, int yPassed,
                                  size_t history_size);

private:
  int checkIndent(const std::vector<char32_t>& raw, size_t i) const;
  bool nextIsValid(Utf8String str, int i);
  int offset(int i);
  bool hasEnding(Utf8String const &fullString, Utf8String const &ending);
};

#endif
