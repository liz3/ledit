#ifndef CURSOR_H
#define CURSOR_H

#include "font_atlas.h"
#include "selection.h"
#include "utf8String.h"
#include <deque>
#include <map>
#include <string>
#include <unordered_map>
#include <vector>
#include <filesystem>
namespace fs = std::filesystem;
struct PosEntry {
  int x, y, skip;
};
struct FoldEntry {
  std::vector<Utf8String> lines;
  uint32_t idx;
};
struct HistoryEntry {
  int x, y;
  int mode;
  int length;
  void *userData;
  Utf8String content;
  std::vector<Utf8String> extra;
};
struct DirEntry {
  fs::path base, entry, full;
  bool isDir = false;
};
struct CommentEntry {
  int firstOffset;
  int yStart;
  Utf8String commentStr;
};
class Cursor {
public:
  bool edited = false;
  bool streamMode = false;
  bool useXFallback = false;
  bool isFolder = false;
  std::string branch;
  std::vector<Utf8String> lines;
  std::unordered_map<int, FoldEntry> foldEntries;
  std::map<std::string, PosEntry> saveLocs;
  std::unordered_map<size_t, DirEntry> dirEntries;
  std::deque<HistoryEntry> history;
  std::filesystem::file_time_type last_write_time;
  Selection selection;
  int x = 0;
  int y = 0;
  int xSave = 0;
  int skip = 0;
  int xOffset = 0;
  float xSkip = 0;
  float height = 0;
  float lineHeight = 0;
  float maxWidth = 0;
  int maxLines = 0;
  int totalCharOffset = 0;
  int cachedY = 0;
  int cachedX = 0;
  int cachedMaxLines = 0;
  float startX = 0;
  float startY = 0;
  size_t idx_c = 1;
  std::vector<std::pair<int, Utf8String>> prepare;
  Utf8String *bind = nullptr;

  Cursor();
  Cursor(std::string path);

  void setBounds(float height, float lineHeight);
  void setBoundsDirect(int next);
  void trimTrailingWhiteSpaces();
  void foldWithCount(size_t count);
  void unfold();
  Utf8String fold();
  size_t getFoldOffset(int off);
  void comment(Utf8String commentStr);
  void resetCursor();
  void setRenderStart(float x, float y);
  void setPosFromMouse(float mouseX, float mouseY, FontAtlas *atlas,
                       bool lineWrapping);
  void reset();
  void deleteSelection();
  std::string getSelection();
  int getSelectionSize();
  void bindTo(Utf8String *entry, bool useXSave = false);
  void unbind();
  Utf8String search(Utf8String what, bool skipFirst, bool shouldOffset = true);

  Utf8String replaceOne(Utf8String what, Utf8String replace,
                        bool allowCenter = true, bool shouldOffset = true);
  size_t replaceAll(Utf8String what, Utf8String replace);

  int findAnyOf(Utf8String str, Utf8String what);
  void jumpMatching(const Utf8String &stringCharacters, char32_t escapeChar);
  std::pair<int, int> findMatchingWithCoords(int inx, int iny,
                                             const Utf8String &stringCharacters,
                                             char32_t escapeChar);
  int findAnyOfLast(Utf8String str, Utf8String what);
  int findAnyOfLastInclusive(Utf8String str, Utf8String what);
  int findAnyOfInclusive(Utf8String str, Utf8String what);
  std::pair<int, int> findGlobal(bool backwards, Utf8String what, int inx,
                                 int iny);

  void advanceWord(bool end = false);
  std::pair<float, float> getPosLineWrapped(FontAtlas &atlas, float xBase,
                                            float yBase, float maxRenderWidth,
                                            float lineHeight, int x, int y);

  void setCurrent(char32_t character);
  int getMaxLinesWrapped(FontAtlas &atlas, float xBase, float yBase,
                         float maxRenderWidth, float lineHeight, float height);
  int indent(std::map<int, int> &map, int start, int end, Utf8String prefix);
  Utf8String clearLine();
  Utf8String deleteLines(int64_t start, int64_t am, bool del = true);
  int getFirstNonWhitespace(int y);
  Utf8String deleteWord();
  Utf8String deleteWordVim(bool withSpace, bool del = true);
  Utf8String deleteWordBackwards(bool onlyCopy = false);
  bool undo();
  void advanceWordBackwards();
  void gotoLine(int l);
  void center(int l);
  std::vector<Utf8String> split(Utf8String base, Utf8String delimiter);
  std::vector<std::string> split(std::string base, std::string delimiter);
  std::vector<std::string> splitNewLine(std::string *base);
  void loadFolder(std::string path);
  DirEntry *getActiveDirEntry();
  void historyPush(int mode, int length, Utf8String content);
  void historyPush(int mode, int length, Utf8String content, void *userData);
  void historyPushWithExtra(int mode, int length, Utf8String content,
                            std::vector<Utf8String> extra);
  bool didChange(std::string path);
  bool reloadFile(std::string path);
  bool openFile(const std::string& oldPath, const std::string& path);
  void append(char32_t c);
  void appendWithLines(const Utf8String& content, bool isVim = false);
  void append(const Utf8String& content);
  Utf8String getCurrentAdvance(bool useSaveValue = false);
  char32_t removeBeforeCursor();
  char32_t removeOne(bool copyOnly = false);

  void moveUp();
  void moveDown();
  void jumpStart();
  void jumpEnd();
  void moveRight();
  void moveLeft();

  const int64_t getCurrentLineLength();
  char32_t getCurrentChar();
  bool saveTo(std::string path);
  std::vector<std::pair<int, Utf8String>> *getContent(FontAtlas *atlas,
                                                      float maxWidth,
                                                      bool onlyCalculate,
                                                      bool lineWrapping);
  void moveLine(int diff);
  void calcTotalOffset();
  int getTotalOffset(); 
  std::vector<std::string> getSaveLocKeys();
};
#endif
