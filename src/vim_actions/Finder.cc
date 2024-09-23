#include "../vim_actions.h"
ActionResult Finder::execute(VimMode mode, MotionState &state, Cursor *cursor,
                       Vim *vim){
    return {};
  }
ActionResult Finder::peek(VimMode mode, MotionState &state, Cursor *cursor,
                    Vim *vim){
    return {};
  }
void Finder::find(char32_t w, bool backwards, bool before, Cursor *cursor, Vim *vim){
    this->active = true;
    this->cc = w;
    this->backwards = backwards;
    this->before = before;
    this->cursor = cursor;
    this->vim = vim;
    foundIndex = -1;
    next();
  }
void Finder::next(bool prev){
    if (!vim || !cursor || cursor != vim->getState().cursor)
      return;
    auto x = cursor->x;
    auto y = cursor->y;
    if ((!backwards && prev) || (backwards && !prev)) {
      if (x == 0)
        return;
      for (int64_t i = x - 1; i >= 0; i--) {
        if (foundIndex != -1 && before && i == foundIndex)
          continue;
        char32_t current = cursor->lines[y][i];
        if (current == cc) {
          foundIndex = i;
          if (before)
            i++;
          cursor->x = i;
          cursor->selection.diffX(i);
          break;
        }
      }
    } else {
      x++;
      for (int64_t i = x + 1; i < cursor->lines[y].size(); i++) {

        if (foundIndex != -1 && before && i == foundIndex)
          continue;
        char32_t current = cursor->lines[y][i];
        if (current == cc) {
          foundIndex = i;
          if (before)
            i--;
          cursor->x = i;
          cursor->selection.diffX(i);
          break;
        }
      }
    }
  }
