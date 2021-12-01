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
class State {
 public:
  GLuint vao, vbo;
  GLuint sel_vao, sel_vbo;
  Cursor* cursor;
  Highlighter highlighter;
  Provider provider;
  float WIDTH, HEIGHT;
  bool hasHighlighting;
  std::string path;
  std::string fileName;
  std::string status;
  std::string miniBuf;
  double lastStroke;
  int mode = 0;
  int round = 0;
  State() {}
  void toggleSelection() {
    if(cursor->selection.active)
      cursor->selection.stop();
    else
      cursor->selection.activate(cursor->x, cursor->y);
  }
  void switchBuffer() {
    if(mode != 0  && mode != 5)
      return;
    if(mode == 0) {
      if(cursor->saveLocs.size() == 0) {
        status = "No other buffers in cache";
        return;
      }
      round = 0;
      miniBuf = cursor->getSaveLocKeys()[0];
      cursor->bindTo(&miniBuf);
      mode = 5;
      status = "Switch to: ";
    } else {
      round++;
      if(round == cursor->saveLocs.size())
        round = 0;
      miniBuf = cursor->getSaveLocKeys()[round];
    }
  }
  void save() {
    if(mode != 0)
      return;
    if(!path.length()) {
      saveNew();
      return;
    }
    cursor->saveTo(path);
    status = "Saved: " + path;
  }
  void saveNew() {
    if(mode != 0)
      return;
    miniBuf = "";
    cursor->bindTo(&miniBuf);
    mode = 1;
    status = "Save to: ";
  }
  void open() {
    if(mode != 0)
      return;
    miniBuf = "";
    provider.lastProvidedFolder = "";
    cursor->bindTo(&miniBuf);
    mode = 4;
    status = "Open: ";
  }
  void reHighlight() {
  if(hasHighlighting)
    highlighter.highlight(cursor->lines);

  }
  void undo() {
    bool result = cursor->undo();
    status = result ? "Undo" : "Undo failed";
    if(result)
      reHighlight();
  }
  void search() {
    if(mode != 0)
      return;
    miniBuf = "";
    cursor->bindTo(&miniBuf);
    mode = 2;
    status = "Search: ";
  }
  void inform(bool success) {
    if(success) {
      if(mode == 1) { // save to
        bool result = cursor->saveTo(miniBuf);
        if(result) {
        status = "Saved to: " + miniBuf;
        if(!path.length()) {
          path = miniBuf;
          auto split = cursor->split(path, "/");
          fileName = split[split.size() -1];
        }
        } else {
          status = "Failed to save to: " + miniBuf;
        }
      } else if (mode == 2 || mode == 7) { // search
        status = cursor->search(miniBuf, false, mode != 7);
        if(mode == 7)
          mode = 2;
        // hacky shit
        if(status != "[Not found]: ")
          mode = 6;
        return;
      } else if (mode == 6) { // search
        status = cursor->search(miniBuf, true);
        if(status == "[No further matches]: ") {
          mode=7;
        }
        return;
      } else if (mode == 3) { // gotoline
        cursor->gotoLine(std::stoi(miniBuf));
        status = "Jump to: " + miniBuf;
      } else if (mode == 4 || mode == 5) {
        bool result = cursor->openFile(path, miniBuf);
        if(result) {
          path = miniBuf;
          auto split = cursor->split(path, "/");
          fileName = split[split.size() -1];
          std::vector<std::string> fileParts = cursor->split(fileName, ".");
          std::string ext = fileParts[fileParts.size()-1];
          const Language* lang = has_language(ext);
          if(lang) {
            highlighter.setLanguage(*lang, lang->modeName);
            highlighter.highlight(cursor->lines);
            hasHighlighting = true;
          } else {
            hasHighlighting = false;
          }

          status = "Loaded: " + miniBuf;
        } else {
          status = "Failed to load: " + miniBuf;
        }
      }
    } else {
      status = "Aborted";
    }
    cursor->unbind();
    mode = 0;
  }
  void provideComplete() {
    if (mode == 4) {
      std::string e = provider.getFileToOpen(miniBuf == provider.getLast() ? provider.lastProvidedFolder : miniBuf);
      if(!e.length())
        return;
      std::string p = provider.lastProvidedFolder;
      miniBuf = e;
    }
  }
  void renderCoords(){
    if(mode != 0)
      return;
    highlighter.highlight(cursor->lines);
    status = std::to_string(cursor->y +1)  + ":" + std::to_string(cursor->x +1) + " ["  + fileName + ": " + (hasHighlighting ? highlighter.languageName : "Text")  + "] History Size: " + std::to_string(cursor->history.size());
    if(cursor->selection.active)
      status += " Selected: [" + std::to_string(cursor->getSelectionSize()) + "]";
  }
  void gotoLine() {
    if(mode != 0)
      return;
    miniBuf = "";
    cursor->bindTo(&miniBuf);
    mode = 3;
    status = "Line: ";


  }
  State(Cursor* c, float w, float h, std::string path) {
    status = path;
    lastStroke = 0;
    cursor = c;
    WIDTH = w;
    HEIGHT = h;
    this->path = path;
    if(path.length()) {
      auto split = cursor->split(path, "/");
      fileName = split[split.size() -1];
      std::vector<std::string> fileParts = cursor->split(fileName, ".");
      std::string ext = fileParts[fileParts.size()-1];
      const Language* lang = has_language(ext);
      if(lang) {
        highlighter.setLanguage(*lang, lang->modeName);
        highlighter.highlight(cursor->lines);
        hasHighlighting = true;
      } else {
        hasHighlighting = false;
      }
    } else {
      fileName = "New File";
      hasHighlighting = false;
      renderCoords();
    }

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
