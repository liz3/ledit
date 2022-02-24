#ifndef DEFS_H_H
#define DEFS_H_H

#include <vector>
#include <map>
#include <string>
#include <fstream>
#include <sstream>
#include "shader.h"
#include "cursor.h"
#include "highlighting.h"
#include "languages.h"
#include "providers.h"
#include "u8String.h"
struct CursorEntry {
  Cursor cursor;
  std::string path;
};
struct ReplaceBuffer {
  std::u16string search = u"";
  std::u16string replace = u"";
};
class State {
 public:
  GLuint vao, vbo;
  bool focused = true;
  bool exitFlag = false;
  GLuint sel_vao, sel_vbo;
  GLuint highlight_vao, highlight_vbo;
  Cursor* cursor;
  std::vector<CursorEntry*> cursors;
  size_t activeIndex;
  Highlighter highlighter;
  Provider provider;
  FontAtlas* atlas;
  GLFWwindow* window;
  ReplaceBuffer replaceBuffer;
  float WIDTH, HEIGHT;
  bool hasHighlighting;
  bool ctrlPressed = false;
  std::string path;
  std::u16string fileName;
  std::u16string status;
  std::u16string miniBuf;
  std::u16string dummyBuf;
  double lastStroke;
  bool showLineNumbers = true;
  bool highlightLine = true;
  int mode = 0;
  int round = 0;
  int fontSize;
  State() {}
  CursorEntry* hasEditedBuffer() {
    for(CursorEntry* cur : cursors) {
      if(cur->cursor.edited)
        return cur;
    }
    return nullptr;
  }
  void startReplace() {
    if(mode != 0)
      return;
    mode = 30;
    status = u"Search: ";
    miniBuf = replaceBuffer.search;
    cursor->bindTo(&miniBuf);

  }
  void tryComment() {
    if(!hasHighlighting)
      return;
    cursor->comment(highlighter.language.singleLineComment);
  }
  void checkChanged() {
    if(!path.length())
      return;
    cursor->branch = provider.getBranchName(path);
    auto changed = cursor->didChange(path);
    if(changed) {
      miniBuf = u"";
      mode = 36;
      status = u"[" + create(path) + u"]: Changed on disk, reload?";
      cursor->bindTo(&dummyBuf);
    }
  }
  void switchMode() {
     if(mode != 0)
       return;
     round = 0;
     miniBuf = u"Text";
     status = u"Mode: ";
     cursor->bindTo(&dummyBuf);
     mode = 25;
  }
  void increaseFontSize(int value) {
    if(mode != 0)
      return;
    fontSize += value;
    if(fontSize > 260) {
      fontSize = 260;
      status = u"Max font size reached [260]";
      return;
    } else if (fontSize < 10) {
      fontSize = 10;
      status = u"Min font size reached [10]";
      return;
    } else {
      status = u"resize: [" + numberToString(fontSize) + u"]";
    }
    atlas->renderFont(fontSize);
  }
  void toggleSelection() {
    if(mode != 0)
      return;
    if(cursor->selection.active)
      cursor->selection.stop();
    else
      cursor->selection.activate(cursor->x, cursor->y);
    renderCoords();
  }
  void switchBuffer() {
    if(mode != 0  && mode != 5)
      return;
    if(mode == 0) {
      if(cursors.size() == 1) {
        status = u"No other buffers in cache";
        return;
      }
      round = 0;
      miniBuf = create(cursors[0]->path);
      cursor->bindTo(&miniBuf);
      mode = 5;
      status = u"Switch to: ";
    } else {
      round++;
      if(round == cursors.size())
        round = 0;
      miniBuf = create(cursors[round]->path);
    }
  }
  void tryPaste() {
    const char* contents = glfwGetClipboardString(NULL);
    if(contents) {
      std::u16string str = create(std::string(contents));
      cursor->appendWithLines(str);
      if(mode != 0)
        return;
      if(hasHighlighting)
        highlighter.highlight(cursor->lines, &provider.colors, cursor->skip, cursor->maxLines, cursor->y);
      status = u"Pasted " + numberToString(str.length()) + u" Characters";
    }
  }
  void cut() {
    if(!cursor->selection.active) {
      status = u"Aborted: No selection";
      return;
    }
    std::string content = cursor->getSelection();
    glfwSetClipboardString(NULL, content.c_str());
    cursor->deleteSelection();
    cursor->selection.stop();
    status = u"Cut " + numberToString(content.length()) + u" Characters";
  }
  void tryCopy() {
    if(!cursor->selection.active) {
      status = u"Aborted: No selection";
      return;
    }
    std::string content = cursor->getSelection();
    glfwSetClipboardString(NULL, content.c_str());
    cursor->selection.stop();
    status = u"Copied " + numberToString(content.length()) + u" Characters";
  }
  void save() {
    if(mode != 0)
      return;
    if(!path.length()) {
      saveNew();
      return;
    }
    cursor->saveTo(path);
    status = u"Saved: " + create(path);
  }
  void saveNew() {
    if(mode != 0)
      return;
    miniBuf = u"";
    cursor->bindTo(&miniBuf);
    mode = 1;
    status = u"Save to[" + create(provider.getCwdFormatted()) + u"]: ";
  }
  void changeFont() {
    if(mode != 0)
      return;
    miniBuf = create(provider.fontPath);
    cursor->bindTo(&miniBuf);
    mode = 15;
    status = u"Set font: ";

  }
  void open() {
    if(mode != 0)
      return;
    miniBuf = u"";
    provider.lastProvidedFolder = "";
    cursor->bindTo(&miniBuf);
    mode = 4;
    status = u"Open [" + create(provider.getCwdFormatted()) + u"]: ";
  }
  void reHighlight() {
  if(hasHighlighting)
    highlighter.highlight(cursor->lines, &provider.colors, cursor->skip, cursor->maxLines, cursor->y);

  }
  void undo() {
    bool result = cursor->undo();
    status = result ? u"Undo" : u"Undo failed";
    if(result)
      reHighlight();
  }
  void search() {
    if(mode != 0)
      return;
    miniBuf = u"";
    cursor->bindTo(&miniBuf);
    mode = 2;
    status = u"Search: ";
  }
  void tryEnableHighlighting() {
    std::vector<std::u16string> fileParts = cursor->split(fileName, u".");
    std::string ext = convert_str(fileParts[fileParts.size()-1]);
    const Language* lang = has_language(fileName == u"Dockerfile" ? "dockerfile" : ext);
    if(lang) {
      highlighter.setLanguage(*lang, lang->modeName);
      highlighter.highlight(cursor->lines, &provider.colors, cursor->skip, cursor->maxLines, cursor->y);
      hasHighlighting = true;
    } else {
      hasHighlighting = false;
    }

  }
  void inform(bool success, bool shift_pressed) {
    if(success) {
      if(mode == 1) { // save to
        bool result = cursor->saveTo(convert_str(miniBuf));
        if(result) {
        status = u"Saved to: " + miniBuf;
        if(!path.length()) {
          path = convert_str(miniBuf);
          cursors[activeIndex]->path = path;
          auto split = cursor->split(path, "/");
          std::string fName = split[split.size() -1];
          fileName = create(fName);
          std::string window_name = "ledit: " + path;
          if(window != NULL) {
            glfwSetWindowTitle(window, window_name.c_str());
          }
          tryEnableHighlighting();
        }
        } else {
          status = u"Failed to save to: " + miniBuf;
        }
      } else if (mode == 2 || mode == 7) { // search
        status = cursor->search(miniBuf, false, mode != 7);
        if(mode == 7)
          mode = 2;
        // hacky shit
        if(status != u"[Not found]: ")
          mode = 6;
        return;
      } else if (mode == 6) { // search
        status = cursor->search(miniBuf, true);
        if(status == u"[No further matches]: ") {
          mode=7;
        }
        return;
      } else if (mode == 3) { // gotoline
        auto line_str = convert_str(miniBuf);
        if(isSafeNumber(line_str)) {
          cursor->gotoLine(std::stoi(line_str));
          status = u"Jump to: " + miniBuf;
        } else {
          status = u"Invalid line: " + miniBuf;
        }
      } else if (mode == 4 || mode == 5) {
        cursor->unbind();
        if(mode == 5) {
          if(round != activeIndex) {
            activateCursor(round);
            status = u"Switched to: " + create(path);
          } else {
            status = u"Canceled";
          }
        } else {
          bool found = false;
          size_t fIndex = 0;
          auto converted = convert_str(miniBuf);
          for(size_t i = 0; i < cursors.size(); i++) {
            if(cursors[i]->path == converted) {
              found = true;
              fIndex = i;
              break;
            }
          }
          if(found && activeIndex != fIndex)
            activateCursor(fIndex);
          else if(!found)
            addCursor(converted);
        }

      } else if (mode == 15) {
         atlas->readFont(convert_str(miniBuf), fontSize);
         provider.fontPath = convert_str(miniBuf);
         provider.writeConfig();
         status = u"Loaded font: " + miniBuf;
      } else if (mode == 25) {
          if(round == 0) {
             status = u"Mode: Text";
             hasHighlighting = false;
          } else {
             auto lang = LANGUAGES[round-1];
             highlighter.setLanguage(lang, lang.modeName);
             hasHighlighting = true;
             status = u"Mode: " + miniBuf;
          }
      } else if (mode == 30) {
        replaceBuffer.search = miniBuf;
        miniBuf = replaceBuffer.replace;
        cursor->unbind();
        cursor->bindTo(&miniBuf);
        status = u"Replace: ";
        mode = 31;
        return;
      } else if (mode == 31) {
        mode = 32;
        replaceBuffer.replace = miniBuf;
        status = replaceBuffer.search + u" => " + replaceBuffer.replace;
        cursor->unbind();
        return;
      } else if(mode == 32) {
        if(shift_pressed) {
          auto count = cursor->replaceAll(replaceBuffer.search, replaceBuffer.replace);
          if(count)
            status = u"Replaced " + numberToString(count) + u" matches";
          else
            status = u"[No match]: " + replaceBuffer.search + u" => " + replaceBuffer.replace;
        } else {
          auto result = cursor->replaceOne(replaceBuffer.search, replaceBuffer.replace, true);
          status = result  + replaceBuffer.search + u" => " + replaceBuffer.replace;
          return;
        }
      } else if (mode == 36) {
           cursor->reloadFile(path);
           status = u"Reloaded";
      }
    } else {
      status = u"Aborted";
    }
    cursor->unbind();
    mode = 0;
  }
  void provideComplete(bool reverse) {
    if (mode == 4 || mode == 15 || mode == 1) {
      std::string convert = convert_str(miniBuf);
      std::string e = provider.getFileToOpen(convert == provider.getLast() ? provider.lastProvidedFolder : convert, reverse);
      if(!e.length())
        return;
      std::string p = provider.lastProvidedFolder;
      miniBuf = create(e);
    } else if (mode == 25) {
       if(reverse) {
          if(round == 0)
            round = LANGUAGES.size();
          else
            round--;
        } else {
       if(round == LANGUAGES.size())
         round = 0;
       else
         round++;
      }
      if(round == 0)
        miniBuf = u"Text";
      else
        miniBuf = create(LANGUAGES[round-1].modeName);
    }
  }
  void renderCoords(){
    if(mode != 0)
      return;
    // if(hasHighlighting)
    // highlighter.highlight(cursor->lines, &provider.colors, cursor->skip,  cursor->maxLines, cursor->y);
    std::u16string branch;
    if(cursor->branch.length()) {
      branch = u" [git: " + create(cursor->branch) + u"]";
    }
    status = numberToString(cursor->y +1)  + u":" + numberToString(cursor->x +1) + branch + u" ["  + fileName + u": " + (hasHighlighting ? highlighter.languageName : u"Text")  + u"] History Size: " + numberToString(cursor->history.size());
    if(cursor->selection.active)
      status += u" Selected: [" + numberToString(cursor->getSelectionSize()) + u"]";
  }
  void gotoLine() {
    if(mode != 0)
      return;
    miniBuf = u"";
    cursor->bindTo(&miniBuf);
    mode = 3;
    status = u"Line: ";


  }
  std::u16string getTabInfo() {
    if(activeIndex == 0 && cursors.size() == 1)
      return u"[ 1 ]";
    std::u16string text = u"[ " + numberToString(activeIndex+1) + u":" + numberToString(cursors.size()) + u" ]";

    return text;
  }

  void deleteActive() {
    deleteCursor(activeIndex);
  }
  void rotateBuffer() {
    if(cursors.size() == 1)
      return;
    size_t next = activeIndex + 1;
    if(next == cursors.size())
      next = 0;
    activateCursor(next);
  }
  void deleteCursor(size_t index) {
    if(mode != 0 || cursors.size() == 1 || index >= cursors.size())
      return;
    CursorEntry* entry = cursors[index];
    delete entry;

    if(activeIndex != index)
      return;
    size_t targetIndex = index == 0 ? 1 : index-1;
    cursors.erase(cursors.begin()+index);
    activateCursor(targetIndex);
  }
  void activateCursor(size_t cursorIndex) {
    CursorEntry* entry = cursors[cursorIndex];
    activeIndex = cursorIndex;
    std::string path= entry->path;
    this->cursor = &(entry->cursor);
    this->path = path;
    status = create(path);
    if(path.length()) {
      if(path == "-") {
        fileName = u"-(STDIN/OUT)";
        hasHighlighting = false;
        renderCoords();
      } else {
        auto split = entry->cursor.split(path, "/");
        fileName = create(split[split.size() -1]);
        tryEnableHighlighting();
        checkChanged();
      }
    } else {
      fileName = u"New File";
      hasHighlighting = false;
      renderCoords();
    }
    std::string window_name = "ledit: " + path;
    if(window!=NULL) {
      glfwSetWindowTitle(window, window_name.c_str());
    }

  }
  void addCursor(std::string path) {
    if(path.length() && std::filesystem::is_directory(path))
      path = "";

    Cursor newCursor = path.length() ? Cursor(path) : Cursor();
    if(path.length()) {
      newCursor.branch = provider.getBranchName(path);
    }
    CursorEntry* entry = new CursorEntry {newCursor, path};
    cursors.push_back(entry);
    activateCursor(cursors.size()-1);
  }
  State(float w, float h, std::string path, int fontSize) {

    this->fontSize = fontSize;
    lastStroke = 0;
    WIDTH = w;
    HEIGHT = h;
    addCursor(path);
  }
  void init() {
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    //TODO set size of vbo base on buffer size?

    glBufferData(GL_ARRAY_BUFFER, sizeof(RenderChar) * 600 * 1000, nullptr, GL_DYNAMIC_DRAW);
    activate_entries();
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // //selection buffer;
    glGenVertexArrays(1, &sel_vao);
    glGenBuffers(1, &sel_vbo);
    glBindVertexArray(sel_vao);
    glBindBuffer(GL_ARRAY_BUFFER, sel_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(SelectionEntry) * 16, nullptr, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(SelectionEntry), (void*)offsetof(SelectionEntry, pos));
    glVertexAttribDivisor(0, 1);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(SelectionEntry), (void*)offsetof(SelectionEntry, size));
    glVertexAttribDivisor(1, 1);


    // //selection buffer;
    glGenVertexArrays(1, &highlight_vao);
    glGenBuffers(1, &highlight_vbo);
    glBindVertexArray(highlight_vao);
    glBindBuffer(GL_ARRAY_BUFFER, highlight_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(SelectionEntry) , nullptr, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(SelectionEntry), (void*)offsetof(SelectionEntry, pos));
    glVertexAttribDivisor(0, 1);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(SelectionEntry), (void*)offsetof(SelectionEntry, size));
    glVertexAttribDivisor(1, 1);


    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);


  }
private:
  void activate_entries() {
    //pos
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(RenderChar), (void*)offsetof(RenderChar, pos));
    glVertexAttribDivisor(0, 1);

    //size
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(RenderChar), (void*)offsetof(RenderChar, size));
    glVertexAttribDivisor(1, 1);

    //uv_pos
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(RenderChar), (void*)offsetof(RenderChar, uv_pos));
    glVertexAttribDivisor(2, 1);
    //uv_size
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, sizeof(RenderChar), (void*)offsetof(RenderChar, uv_size));
    glVertexAttribDivisor(3, 1);
    //fg_color
    glEnableVertexAttribArray(4);
    glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(RenderChar), (void*)offsetof(RenderChar, fg_color));
    glVertexAttribDivisor(4, 1);
    //bg_color
    glEnableVertexAttribArray(5);
    glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, sizeof(RenderChar), (void*)offsetof(RenderChar, bg_color));
    glVertexAttribDivisor(5, 1);

  }

};

#endif
