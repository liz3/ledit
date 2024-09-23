#include "../vim_actions.h"
ActionResult IndentAction::execute(VimMode mode, MotionState &state, Cursor *cursor,
                       Vim *vim){
    int res = 0;
    State &st = vim->getState();
    auto useSpaces = st.provider.useSpaces;
    auto tabWidth = st.provider.tabWidth;
    if (st.hasHighlighting) {
      auto pref = useSpaces ? Utf8String(std::string(tabWidth, ' '))
                            : Utf8String(std::string(1, '\t'));
      auto &m = st.highlighter.indentLevels;
      if (state.isInital) {
        res = cursor->indent(m, cursor->y, cursor->y + 1, pref);

      } else if (state.action.length() == 1) {
        for (auto &pair : PAIRS) {
          if (state.action[0] == pair.first || state.action[0] == pair.second) {
            Utf8String in(state.action);
            bool isClosing = state.action[0] == pair.second;

            auto result =
                cursor->findGlobal(!isClosing, in, cursor->x, cursor->y);
            if (result.first == -1 && result.second == -1) {
              return {};
            }
            cursor->x = result.first;

            cursor->y = result.second;

            if (state.replaceMode == ReplaceMode::ALL && !isClosing)
              cursor->selection.activate(cursor->x == 0 ? 0 : cursor->x,
                                         cursor->y);
            else
              cursor->selection.activate(state.replaceMode == ReplaceMode::ALL
                                             ? cursor->x + 1
                                             : cursor->x,
                                         cursor->y);
            {
              if (st.hasHighlighting)
                cursor->jumpMatching(st.highlighter.language.stringCharacters,
                                     st.highlighter.language.escapeChar);
              else {
                cursor->jumpMatching(U"\"", '\\');
              }
            }
            res = cursor->indent(m, cursor->selection.getYSmaller(),
                                 cursor->selection.getYBigger(), pref);
            cursor->selection.stop();
            break;
          }
        }
      } else if (state.direction == Direction::UP ||
                 state.direction == Direction::DOWN) {
        if (state.count) {
          if (state.direction == Direction::UP) {
            res = cursor->indent(m, cursor->y - state.count, cursor->y, pref);
          } else {
            res = cursor->indent(m, cursor->y, cursor->y + state.count, pref);
          }
        }
      }
    }
    if (res) {
      st.status = U"Indented: " + Utf8String(std::to_string(res));
      ActionResult r = {};
      r.allowCoords = false;
      return r;
    }
    return {};
  }
ActionResult IndentAction::peek(VimMode mode, MotionState &state, Cursor *cursor,
                    Vim *vim){
    if (vim->activeAction() == nullptr && mode == VimMode::NORMAL)
      return withType(ResultType::SetSelf);
    if (vim->activeAction() == nullptr && mode == VimMode::VISUAL) {
      State &st = vim->getState();
      auto useSpaces = st.provider.useSpaces;
      auto tabWidth = st.provider.tabWidth;
      if (st.hasHighlighting) {
        auto pref = useSpaces ? Utf8String(std::string(tabWidth, ' '))
                              : Utf8String(std::string(1, '\t'));
        int res = cursor->indent(st.highlighter.indentLevels,
                                 cursor->selection.getYSmaller(),
                                 cursor->selection.getYBigger(), pref);
        if (res) {
          st.status = U"Indented: " + Utf8String(std::to_string(res));
          ActionResult r = {};
          r.allowCoords = false;
          return r;
        }
      }
    }
    if (vim->activeAction() && vim->activeAction()->action_name == "=")
      return execute(mode, state, cursor, vim);
    return {};
  }
