#include "highlighting.h"
#include "u8String.h"
#include <algorithm>

bool Highlighter::isNonChar(char32_t c) { return language.whitespace.count(c); }
bool Highlighter::isNumber(char32_t c) { return c >= '0' && c <= '9'; }
bool Highlighter::isNumberEnd(char32_t c, bool hexa) {
  if (hexa && ((c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F')))
    return false;
  return !isNumber(c) && c != '.' && c != 'x';
}
std::string Highlighter::lowerCase(const std::string &str) {
  std::string out = str;

  std::transform(out.begin(), out.end(), out.begin(),
                 [](unsigned char c) { return std::tolower(c); });
  return out;
}
std::map<int, Vec4f>* Highlighter::get() { return &cached; }
void Highlighter::setLanguage(Language lang, std::string name) {
    language.keywords_case_sensitive = lang.keywords_case_sensitive;
    language.special_case_sensitive = lang.special_case_sensitive;
    language.modeName = create(lang.modeName);
    language.keyWords.clear();
    for (auto &entry : lang.keyWords) {
      if (!lang.keywords_case_sensitive) {
        language.keyWords[lowerCase(entry)] = 1;
      } else {
        language.keyWords[entry] = 1;
      }
    }
    language.specialWords.clear();
    for (auto &entry : lang.specialWords) {
      if (!lang.special_case_sensitive) {
        language.specialWords[lowerCase(entry)] = true;
      } else {
        language.specialWords[entry] = 1;
      }
    }
    if (lang.singleLineComment.length()) {
      language.singleLineComment = create(lang.singleLineComment);
    } else {
      language.singleLineComment = U"";
    }
    if (lang.multiLineComment.first.length()) {
      language.multiLineComment =
          std::pair(create(lang.multiLineComment.first),
                    create(lang.multiLineComment.second));
    } else {
      language.multiLineComment = std::pair(U"", U"");
    }
    language.stringCharacters = create(lang.stringCharacters);
    language.escapeChar = (char32_t)lang.escapeChar;
    Utf8String whitespaceChars(lang.whitespace);
    language.whitespace.clear();
    for (auto c : whitespaceChars) {
      language.whitespace[c] = true;
    }
    language.symbols = Utf8String(lang.symbols);
    language.tabIndent = lang.tabIndent;
    language.indentStr = Utf8String(lang.indentStr);
    language.outdentStr = Utf8String(lang.outdentStr);
    languageName = create(name);
    wasCached = false;
  }
  std::map<int, Vec4f>* Highlighter::highlight(std::vector<Utf8String> &lines,
                                  EditorColors *colors, int skip, int maxLines,
                                  int y, size_t history_size) {
    Utf8String str;
    for (size_t i = 0; i < lines.size(); i++) {
      str += lines[i];
      if (i < lines.size() - 1)
        str += U"\n";
    }
    return highlight(str, colors, skip, maxLines, y, history_size);
  }
std::map<int, Vec4f>* Highlighter::highlight(Utf8String &raw, EditorColors *colors,
                                  int skip, int maxLines, int yPassed,
                                  size_t history_size) {
    if (wasCached && wasEntire && history_size == last_history_size)
      return &cached;

    std::map<int, Vec4f> entries;
    if (skip != lastSkip) {
      wasCached = false;
      wasEntire = true;
    } else {
      wasEntire = !wasCached;
    }

    HighlighterState state = {0, false, false, 0, U"", 0, false};
    int startIndex = 0;
    int y = 0;
    lineIndex.clear();
    indentLevels.clear();
    Vec4f string_color = colors->string_color;
    Vec4f default_color = colors->default_color;
    Vec4f keyword_color = colors->keyword_color;
    Vec4f special_color = colors->special_color;
    Vec4f comment_color = colors->comment_color;
    Vec4f number_color = colors->number_color;
    Vec4f symbol_color = colors->symbol_color;
    char32_t last = 0;
    size_t i;
    size_t lCount = 0;
    int indent = 0;
    int last_entry = -1;
    bool wasTriggered = false;
    auto vec = raw.getCodePoints();
    for (i = 0; i < raw.length(); i++) {
      char32_t current = vec[i];
      int indentRes = checkIndent(vec, i);
      if (!state.busy && indentRes > 0) {
        if (indentRes == 1) {
          indentLevels[y] = indent;
          indent++;
        } else if (indentRes == 2) {
          if(indent > 0)
          indent--;
          indentLevels[y] = indent;
        }
      }
      if (current == '\n') {
        if (!indentLevels.count(y))
          indentLevels[y] = indent;

        if (lCount++ > skip + maxLines && wasCached)
          break;
      }
      if (skip > 0 && wasCached && lCount < skip - 1) {
        if (current == '\n') 
          y++;
        continue;
      }

      if (state.busy && (state.mode == 6 || state.mode == 7) &&
          isNumberEnd(current, state.mode == 7)) {
        state.buffer = U"";
        state.busy = false;
        state.mode = 0;
        entries[i] = default_color;
        last_entry = i;
      }

      if (language.stringCharacters.find(current) != std::string::npos &&
          (last != language.escapeChar ||
           (last == language.escapeChar && i > 1 &&
            raw[i - 2] == language.escapeChar))) {
        if (state.mode == 0 && !state.busy) {
          state.mode = 1;
          state.busy = true;
          state.start = i;
          state.stringChar = current;
          entries[i] = string_color;
          last_entry = i;
        } else if (state.mode == 1 && current == state.stringChar) {
          state.busy = false;
          state.mode = 0;
          state.start = 0;
          entries[i + 1] = default_color;
          last_entry = i + 1;
        }
      } else if (state.busy && state.mode == 3 &&
                 hasEnding(state.buffer, language.multiLineComment.second)) {
        state.mode = 0;
        state.busy = false;
        entries[i] = default_color;
        last_entry = i;
      } else if ((!state.busy || state.mode == 2) &&
                 language.multiLineComment.first.length() &&
                 hasEnding(state.buffer, language.multiLineComment.first)) {
        entries[i - language.multiLineComment.first.length()] = comment_color;
        last_entry = i - (language.multiLineComment.first.length());
        state.buffer = U"";
        state.busy = true;
        state.mode = 3;
      } else if (state.busy && state.mode == 2 && current == '\n') {
        state.buffer = U"";
        state.busy = false;
        state.mode = 0;
        entries[i] = default_color;
        last_entry = i;
      } else if (!state.busy && language.singleLineComment.length() &&
                 hasEnding(state.buffer + current,
                           language.singleLineComment)) {

        entries[i - (language.singleLineComment.length() - 1)] = comment_color;
        last_entry = i - (language.singleLineComment.length() - 1);
        state.busy = true;
        state.mode = 2;
        state.buffer = U"";
      } else if (isNonChar(current) && !state.busy && state.buffer.length() &&
                 !state.wasReset) {
        if (language.keyWords.count(
                language.keywords_case_sensitive
                    ? state.buffer.getStrRef()
                    : lowerCase(state.buffer.getStrRef()))) {

          entries[state.start] = keyword_color;
          entries[i] = default_color;
          last_entry = i;
          state.wasReset = true;
          state.buffer = U"";
        } else if (language.specialWords.count(
                       language.special_case_sensitive
                           ? state.buffer.getStrRef()
                           : lowerCase(state.buffer.getStrRef()))) {
          entries[state.start] = special_color;
          entries[i] = default_color;
          last_entry = i;
          state.wasReset = true;
          state.buffer = U"";
        }
      } else if (isNumber(current) && (isNonChar(last) || state.lastOperator) && !state.busy) {
        state.mode = 6;
        if (current == '0' && i < raw.length() - 1 &&
            (vec[i + 1] == 'x' || vec[i + 1] == 'X'))
          state.mode = 7;
        state.busy = true;
        last_entry = i;
        entries[i] = number_color;
      } else if (!state.busy && isNonChar(last) && !isNonChar(current)) {
        state.wasReset = false;
        state.buffer = U"";
        state.start = i;
      }
      bool isSymbol = !state.lastOperator && !state.busy &&
                      language.symbols.find(current) != std::string::npos;
      if (isSymbol) {
        if (last_entry != i || vec4f_eq(entries[last_entry], default_color)) {
          entries[i] = symbol_color;
          last_entry = i;
          state.lastOperator = true;
        }
      } else if (state.lastOperator &&
                 language.symbols.find(current) == std::string::npos) {
        if (!state.busy && last_entry != i) {
          entries[i] = default_color;
          last_entry = i;
        }
        state.lastOperator = false;
      }
      state.buffer += current;
      last = current;
      if (current == '\n') {
        int endIndex = entries.size();
        lineIndex[y++] = std::pair<int, int>(startIndex, endIndex);
        startIndex = endIndex;
        bool cont = false;
        if (skip > 0) {
          if (lCount == skip) {
            if (!wasCached && skip != lastSkip) {
              savedState = {
                  state, last_entry == -1 ? default_color : entries[last_entry],
                  true};
              entries[i + 1] = savedState.color;
            } else if (wasCached && savedState.loaded) {
              entries[i + 1] = savedState.color;
              state = savedState.state;
            }
          }
        }
      } else if (wasTriggered) {
        wasTriggered = false;
      }
    }

    if (state.buffer.length()) {

      if (language.keyWords.count(language.keywords_case_sensitive
                                      ? state.buffer.getStrRef()
                                      : lowerCase(state.buffer.getStrRef()))) {
        entries[state.start] = keyword_color;
        entries[i] = default_color;
        state.wasReset = true;
      } else if (language.specialWords.count(
                     language.special_case_sensitive
                         ? state.buffer.getStrRef()
                         : lowerCase(state.buffer.getStrRef()))) {
        entries[state.start] = special_color;
        entries[i] = default_color;
        state.wasReset = true;
      } else if (hasEnding(state.buffer, language.singleLineComment)) {

        entries[offset(i) - language.singleLineComment.length()] =
            comment_color;
        state.busy = true;
        state.mode = 2;
        state.buffer = U"";
      } else if (hasEnding(state.buffer, language.multiLineComment.first)) {
        entries[i] = comment_color;
        state.buffer = U"";
        state.busy = true;
        state.mode = 3;
      }
    }
    int endIndex = entries.size();
    lineIndex[y++] = std::pair<int, int>(startIndex, endIndex);

    lastMax = maxLines;
    cached = entries;
    lastY = yPassed;
    last_history_size = history_size;

    wasCached = true;
    lastSkip = skip;
    return &cached;
  }
  int Highlighter::checkIndent(const std::vector<char32_t>& raw, size_t i) const {
    if(!language.indentStr.length() && !language.outdentStr.length())
      return 0;
    auto indentVec = language.indentStr.getCodePoints();
    auto outdentVec = language.outdentStr.getCodePoints();
    int indentOffset = 0;
    int outdentOffset = 0;
    for(size_t s = i; s < raw.size(); s+=1){
        if(indentOffset == -1 && outdentOffset == -1) 
          return 0;
        if(indentOffset != -1 && raw[i] == indentVec[indentOffset++]){
          if(indentOffset == indentVec.size())
            return 1;
        } else {
          indentOffset = -1;
        }
       if(outdentOffset != -1 && raw[i] == outdentVec[outdentOffset++]){
          if(outdentOffset == outdentVec.size())
            return 2;
        } else {
          outdentOffset = -1;
        }
    }
    return 0;
  }
  bool Highlighter::nextIsValid(Utf8String str, int i) {
    return i >= str.length() - 1 || isNonChar(str[i + 1]);
  }
  int Highlighter::offset(int i) { return i + 1; }
    bool Highlighter::hasEnding(Utf8String const &fullString, Utf8String const &ending) {
    if (fullString.length() >= ending.length()) {
      return fullString.endsWith(ending);
    } else {
      return false;
    }
  }