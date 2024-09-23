#ifndef LEDIT_VIM_ACTIONS_H
#define LEDIT_VIM_ACTIONS_H

#include "cursor.h"
#include "state.h"
#include "utf8String.h"
#include "vim.h"
#include "utils.h"

ActionResult withType(ResultType type);
class EscapeAction : public Action {
      public:
      ActionResult execute(VimMode mode, MotionState &state, Cursor *cursor,
                       Vim *vim) override;
ActionResult peek(VimMode mode, MotionState &state, Cursor *cursor,
                    Vim *vim) override;
      private:
      bool test = false;
    };
class BackspaceAction : public Action {
      public:
      ActionResult execute(VimMode mode, MotionState &state, Cursor *cursor,
                       Vim *vim) override;
ActionResult peek(VimMode mode, MotionState &state, Cursor *cursor,
                    Vim *vim) override;
      
    };
class EnterAction : public Action {
      public:
      ActionResult execute(VimMode mode, MotionState &state, Cursor *cursor,
                       Vim *vim) override;
ActionResult peek(VimMode mode, MotionState &state, Cursor *cursor,
                    Vim *vim) override;
void commandParser(Utf8String &buffer, Vim *vim, Cursor *c);
      
    };
class TabAction : public Action {
      public:
      ActionResult execute(VimMode mode, MotionState &state, Cursor *cursor,
                       Vim *vim) override;
ActionResult peek(VimMode mode, MotionState &state, Cursor *cursor,
                    Vim *vim) override;
      
    };
class IAction : public Action {
      public:
      ActionResult execute(VimMode mode, MotionState &state, Cursor *cursor,
                       Vim *vim) override;
ActionResult peek(VimMode mode, MotionState &state, Cursor *cursor,
                    Vim *vim) override;
      
    };
class HAction : public Action {
      public:
      ActionResult execute(VimMode mode, MotionState &state, Cursor *cursor,
                       Vim *vim) override;
ActionResult peek(VimMode mode, MotionState &state, Cursor *cursor,
                    Vim *vim) override;
      
    };
class JAction : public Action {
      public:
      ActionResult execute(VimMode mode, MotionState &state, Cursor *cursor,
                       Vim *vim) override;
ActionResult peek(VimMode mode, MotionState &state, Cursor *cursor,
                    Vim *vim) override;
      
    };
class KAction : public Action {
      public:
      ActionResult execute(VimMode mode, MotionState &state, Cursor *cursor,
                       Vim *vim) override;
ActionResult peek(VimMode mode, MotionState &state, Cursor *cursor,
                    Vim *vim) override;
      
    };
class LAction : public Action {
      public:
      ActionResult execute(VimMode mode, MotionState &state, Cursor *cursor,
                       Vim *vim) override;
ActionResult peek(VimMode mode, MotionState &state, Cursor *cursor,
                    Vim *vim) override;
      
    };
class AAction : public Action {
      public:
      ActionResult execute(VimMode mode, MotionState &state, Cursor *cursor,
                       Vim *vim) override;
ActionResult peek(VimMode mode, MotionState &state, Cursor *cursor,
                    Vim *vim) override;
      
    };
class AAAction : public Action {
      public:
      ActionResult execute(VimMode mode, MotionState &state, Cursor *cursor,
                       Vim *vim) override;
ActionResult peek(VimMode mode, MotionState &state, Cursor *cursor,
                    Vim *vim) override;
      
    };
class WAction : public Action {
      public:
      ActionResult execute(VimMode mode, MotionState &state, Cursor *cursor,
                       Vim *vim) override;
ActionResult peek(VimMode mode, MotionState &state, Cursor *cursor,
                    Vim *vim) override;
      
    };
class BAction : public Action {
      public:
      ActionResult execute(VimMode mode, MotionState &state, Cursor *cursor,
                       Vim *vim) override;
ActionResult peek(VimMode mode, MotionState &state, Cursor *cursor,
                    Vim *vim) override;
      
    };
class OAction : public Action {
      public:
      ActionResult execute(VimMode mode, MotionState &state, Cursor *cursor,
                       Vim *vim) override;
ActionResult peek(VimMode mode, MotionState &state, Cursor *cursor,
                    Vim *vim) override;
      
    };
class OOAction : public Action {
      public:
      ActionResult execute(VimMode mode, MotionState &state, Cursor *cursor,
                       Vim *vim) override;
ActionResult peek(VimMode mode, MotionState &state, Cursor *cursor,
                    Vim *vim) override;
      
    };
class DAction : public Action {
      public:
      ActionResult execute(VimMode mode, MotionState &state, Cursor *cursor,
                       Vim *vim) override;
ActionResult peek(VimMode mode, MotionState &state, Cursor *cursor,
                    Vim *vim) override;
ActionResult copyOnly(VimMode mode, MotionState &state, Cursor *cursor,
                        Vim *vim);
ActionResult markOnly(VimMode mode, MotionState &state, Cursor *cursor,
                        Vim *vim);
      private:
      bool onlyCopy = false;
bool onlyMark = false;
    };
class UAction : public Action {
      public:
      ActionResult execute(VimMode mode, MotionState &state, Cursor *cursor,
                       Vim *vim) override;
ActionResult peek(VimMode mode, MotionState &state, Cursor *cursor,
                    Vim *vim) override;
      
    };
class VAction : public Action {
      public:
      ActionResult execute(VimMode mode, MotionState &state, Cursor *cursor,
                       Vim *vim) override;
ActionResult peek(VimMode mode, MotionState &state, Cursor *cursor,
                    Vim *vim) override;
      
    };
class CAction : public Action {
      public:
      ActionResult execute(VimMode mode, MotionState &state, Cursor *cursor,
                       Vim *vim) override;
ActionResult peek(VimMode mode, MotionState &state, Cursor *cursor,
                    Vim *vim) override;
      
    };
class DollarAction : public Action {
      public:
      DollarAction(bool ctrl_);
ActionResult execute(VimMode mode, MotionState &state, Cursor *cursor,
                       Vim *vim) override;
ActionResult peek(VimMode mode, MotionState &state, Cursor *cursor,
                    Vim *vim) override;
      private:
      bool ctrl = false;
    };
class ZeroAction : public Action {
      public:
      ZeroAction(bool ctrl_);
ActionResult execute(VimMode mode, MotionState &state, Cursor *cursor,
                       Vim *vim) override;
ActionResult peek(VimMode mode, MotionState &state, Cursor *cursor,
                    Vim *vim) override;
      private:
      bool ctrl = false;
    };
class ColonAction : public Action {
      public:
      ActionResult execute(VimMode mode, MotionState &state, Cursor *cursor,
                       Vim *vim) override;
ActionResult peek(VimMode mode, MotionState &state, Cursor *cursor,
                    Vim *vim) override;
      
    };
class YAction : public Action {
      public:
      YAction(DAction *d);
ActionResult execute(VimMode mode, MotionState &state, Cursor *cursor,
                       Vim *vim) override;
ActionResult peek(VimMode mode, MotionState &state, Cursor *cursor,
                    Vim *vim) override;
      private:
      DAction *d = nullptr;
    };
class PAction : public Action {
      public:
      ActionResult execute(VimMode mode, MotionState &state, Cursor *cursor,
                       Vim *vim) override;
ActionResult peek(VimMode mode, MotionState &state, Cursor *cursor,
                    Vim *vim) override;
      
    };
class PercentAction : public Action {
      public:
      ActionResult execute(VimMode mode, MotionState &state, Cursor *cursor,
                       Vim *vim) override;
ActionResult peek(VimMode mode, MotionState &state, Cursor *cursor,
                    Vim *vim) override;
      
    };
class IIAction : public Action {
      public:
      ActionResult execute(VimMode mode, MotionState &state, Cursor *cursor,
                       Vim *vim) override;
ActionResult peek(VimMode mode, MotionState &state, Cursor *cursor,
                    Vim *vim) override;
      
    };
class GGAction : public Action {
      public:
      ActionResult execute(VimMode mode, MotionState &state, Cursor *cursor,
                       Vim *vim) override;
ActionResult peek(VimMode mode, MotionState &state, Cursor *cursor,
                    Vim *vim) override;
      
    };
class GAction : public Action {
      public:
      ActionResult execute(VimMode mode, MotionState &state, Cursor *cursor,
                       Vim *vim) override;
ActionResult peek(VimMode mode, MotionState &state, Cursor *cursor,
                    Vim *vim) override;
      
    };
class ParagraphAction : public Action {
      public:
      ParagraphAction(std::string symbol, DAction *d);
ActionResult execute(VimMode mode, MotionState &state, Cursor *cursor,
                       Vim *vim) override;
ActionResult peek(VimMode mode, MotionState &state, Cursor *cursor,
                    Vim *vim) override;
      private:
      std::string symbol;
DAction *d;
    };
class BracketAction : public Action {
      public:
      BracketAction(std::string symbol, DAction *d);
ActionResult execute(VimMode mode, MotionState &state, Cursor *cursor,
                       Vim *vim) override;
ActionResult peek(VimMode mode, MotionState &state, Cursor *cursor,
                    Vim *vim) override;
      private:
      std::string symbol;
DAction *d;
    };
class ParenAction : public Action {
      public:
      ParenAction(std::string symbol, DAction *d);
ActionResult execute(VimMode mode, MotionState &state, Cursor *cursor,
                       Vim *vim) override;
ActionResult peek(VimMode mode, MotionState &state, Cursor *cursor,
                    Vim *vim) override;
      private:
      std::string symbol;
DAction *d;
    };
class QuoteAction : public Action {
      public:
      QuoteAction(std::string symbol, DAction *d);
ActionResult execute(VimMode mode, MotionState &state, Cursor *cursor,
                       Vim *vim) override;
ActionResult peek(VimMode mode, MotionState &state, Cursor *cursor,
                    Vim *vim) override;
      private:
      std::string symbol;
DAction *d;
    };
class SlashAction : public Action {
      public:
      ActionResult execute(VimMode mode, MotionState &state, Cursor *cursor,
                       Vim *vim) override;
ActionResult peek(VimMode mode, MotionState &state, Cursor *cursor,
                    Vim *vim) override;
      
    };
class FontSizeAction : public Action {
      public:
      FontSizeAction(bool v);
ActionResult execute(VimMode mode, MotionState &state, Cursor *cursor,
                       Vim *vim) override;
ActionResult peek(VimMode mode, MotionState &state, Cursor *cursor,
                    Vim *vim) override;
      private:
      bool increase = false;
    };
class MoveAction : public Action {
      public:
      MoveAction(Direction v);
MoveAction(Direction v, bool ctrl);
ActionResult execute(VimMode mode, MotionState &state, Cursor *cursor,
                       Vim *vim) override;
ActionResult peek(VimMode mode, MotionState &state, Cursor *cursor,
                    Vim *vim) override;
      private:
      Direction direction;
bool need_ctrl = true;
    };
class RAction : public Action {
      public:
      class XInterceptor : public Interceptor {
    ActionResult intercept(char32_t in, Vim *vim, Cursor *cursor) override {
      cursor->setCurrent(in);
      vim->setInterceptor(nullptr);
      return {};
    }
  };


ActionResult execute(VimMode mode, MotionState &state, Cursor *cursor,
                       Vim *vim) override;
ActionResult peek(VimMode mode, MotionState &state, Cursor *cursor,
                    Vim *vim) override;
      private:
      XInterceptor interceptor;
    };
class Finder : public Action {
      public:
      ActionResult execute(VimMode mode, MotionState &state, Cursor *cursor,
                       Vim *vim) override;
ActionResult peek(VimMode mode, MotionState &state, Cursor *cursor,
                    Vim *vim) override;
void find(char32_t w, bool backwards, bool before, Cursor *cursor, Vim *vim);
void next(bool prev = false);
      private:
      bool active = false, backwards = false, before = false;
int foundIndex = -1;
char32_t cc;
Cursor *cursor = nullptr;
Vim *vim = nullptr;
    };
class FindAction : public Action {
      public:
      class FindInterceptor : public Interceptor {
  private:
    FindAction *action = nullptr;

  public:
    FindInterceptor(FindAction *action) { this->action = action; }
    ActionResult intercept(char32_t in, Vim *vim, Cursor *cursor) override {
      action->finder->find(in, action->backwards, action->offset, cursor, vim);
      vim->setInterceptor(nullptr);
      return {};
    }
  };


FindAction(Finder *finder_, bool backwards_, bool offset_);
ActionResult execute(VimMode mode, MotionState &state, Cursor *cursor,
                       Vim *vim) override;
ActionResult peek(VimMode mode, MotionState &state, Cursor *cursor,
                    Vim *vim) override;
      private:
      Finder *finder;
bool backwards;
bool offset;
FindInterceptor *interceptor;
    };
class SemicolonAction : public Action {
      public:
      SemicolonAction(Finder *finder_);
ActionResult execute(VimMode mode, MotionState &state, Cursor *cursor,
                       Vim *vim) override;
ActionResult peek(VimMode mode, MotionState &state, Cursor *cursor,
                    Vim *vim) override;
      private:
      Finder *finder;
    };
class CommaAction : public Action {
      public:
      CommaAction(Finder *finder_);
ActionResult execute(VimMode mode, MotionState &state, Cursor *cursor,
                       Vim *vim) override;
ActionResult peek(VimMode mode, MotionState &state, Cursor *cursor,
                    Vim *vim) override;
      private:
      Finder *finder;
    };
class DDAction : public Action {
      public:
      DDAction(DAction *finder_);
ActionResult execute(VimMode mode, MotionState &state, Cursor *cursor,
                       Vim *vim) override;
ActionResult peek(VimMode mode, MotionState &state, Cursor *cursor,
                    Vim *vim) override;
      private:
      DAction *finder;
    };
class XAction : public Action {
      public:
      XAction(bool control_);
ActionResult execute(VimMode mode, MotionState &state, Cursor *cursor,
                       Vim *vim) override;
ActionResult peek(VimMode mode, MotionState &state, Cursor *cursor,
                    Vim *vim) override;
      private:
      bool control = false;
    };
class CCAction : public Action {
      public:
      ActionResult execute(VimMode mode, MotionState &state, Cursor *cursor,
                       Vim *vim) override;
ActionResult peek(VimMode mode, MotionState &state, Cursor *cursor,
                    Vim *vim) override;
      
    };
class CtrlUAction : public Action {
      public:
      CtrlUAction(bool control_);
ActionResult execute(VimMode mode, MotionState &state, Cursor *cursor,
                       Vim *vim) override;
ActionResult peek(VimMode mode, MotionState &state, Cursor *cursor,
                    Vim *vim) override;
      private:
      bool control = false;
    };
class CommentAction : public Action {
      public:
      ActionResult execute(VimMode mode, MotionState &state, Cursor *cursor,
                       Vim *vim) override;
ActionResult peek(VimMode mode, MotionState &state, Cursor *cursor,
                    Vim *vim) override;
      
    };
class SimpleCopy : public Action {
      public:
      SimpleCopy(bool copy_);
ActionResult execute(VimMode mode, MotionState &state, Cursor *cursor,
                       Vim *vim) override;
ActionResult peek(VimMode mode, MotionState &state, Cursor *cursor,
                    Vim *vim) override;
      private:
      bool copy = false;
    };
class IndentAction : public Action {
      public:
      ActionResult execute(VimMode mode, MotionState &state, Cursor *cursor,
                       Vim *vim) override;
ActionResult peek(VimMode mode, MotionState &state, Cursor *cursor,
                    Vim *vim) override;
      
    };

void register_vim_commands(Vim &vim, State &state);

#endif