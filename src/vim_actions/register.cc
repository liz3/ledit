#include "../vim_actions.h"

ActionResult withType(ResultType type) {
  ActionResult r;
  r.type = type;
  return r;
}
void register_vim_commands(Vim &vim, State &state) {
  Finder *finder = new Finder();
  auto *d = new DAction();
  vim.registerTrie(new EscapeAction(), "ESC", GLFW_KEY_ESCAPE);
  vim.registerTrie(new BackspaceAction(), "BACKSPACE", GLFW_KEY_BACKSPACE);
  vim.registerTrie(new EnterAction(), "ENTER", GLFW_KEY_ENTER);
  vim.registerTrie(new TabAction(), "TAB", GLFW_KEY_TAB);
  vim.registerTrie(new XAction(true), "X_CTRL", GLFW_KEY_D);
  vim.registerTrie(new FontSizeAction(true), "F_INCREASE", GLFW_KEY_EQUAL);
  vim.registerTrie(new FontSizeAction(false), "F_DECREASE", GLFW_KEY_MINUS);
  vim.registerTrie(new TabAction(), "TAB", GLFW_KEY_TAB);
  vim.registerTrie(new CtrlUAction(true), "CTRL+U", GLFW_KEY_U);
  vim.registerTrie(new SimpleCopy(true), "CTRL+C", GLFW_KEY_C);
  vim.registerTrie(new SimpleCopy(false), "CTRL+V", GLFW_KEY_V);
  vim.registerTrie(new CommentAction(), "COMMENT", GLFW_KEY_SLASH);
  vim.registerTrie(new DollarAction(true), "CTRL+E", GLFW_KEY_E);
  vim.registerTrie(new ZeroAction(true), "CTRL+A", GLFW_KEY_A);
  vim.registerTrie(new MoveAction(Direction::UP), "M_UP", GLFW_KEY_P);
  vim.registerTrie(new MoveAction(Direction::RIGHT), "M_RIGHT", GLFW_KEY_F);
  vim.registerTrie(new MoveAction(Direction::DOWN), "M_DOWN", GLFW_KEY_N);
  vim.registerTrie(new MoveAction(Direction::LEFT), "M_LEFT", GLFW_KEY_B);
  vim.registerTrie(new MoveAction(Direction::UP, false), "M_ARROW_UP",
                   GLFW_KEY_UP);
  vim.registerTrie(new MoveAction(Direction::RIGHT, false), "M_ARROW_RIGHT",
                   GLFW_KEY_RIGHT);
  vim.registerTrie(new MoveAction(Direction::DOWN, false), "M_ARROW_DOWN",
                   GLFW_KEY_DOWN);
  vim.registerTrie(new MoveAction(Direction::LEFT, false), "M_ARROW_LEFT",
                   GLFW_KEY_LEFT);
  vim.registerTrieChar(new IAction(), "i", 'i');
  vim.registerTrieChar(new HAction(), "h", 'h');
  vim.registerTrieChar(new JAction(), "j", 'j');
  vim.registerTrieChar(new KAction(), "k", 'k');
  vim.registerTrieChar(new LAction(), "l", 'l');
  vim.registerTrieChar(new AAction(), "a", 'a');
  vim.registerTrieChar(new AAAction(), "A", 'A');
  vim.registerTrieChar(new WAction(), "w", 'w');
  vim.registerTrieChar(new BAction(), "b", 'b');
  vim.registerTrieChar(new OAction(), "o", 'o');
  vim.registerTrieChar(new OOAction(), "O", 'O');
  vim.registerTrieChar(d, "d", 'd');
  vim.registerTrieChar(new DDAction(d), "D", 'D');
  vim.registerTrieChar(new UAction(), "u", 'u');
  vim.registerTrieChar(new XAction(false), "x", 'x');
  vim.registerTrieChar(new VAction(), "v", 'v');
  vim.registerTrieChar(new CAction(), "c", 'c');
  vim.registerTrieChar(new CCAction(), "C", 'C');
  vim.registerTrieChar(new DollarAction(false), "$", '$');
  vim.registerTrieChar(new ZeroAction(false), "0", '0');
  vim.registerTrieChar(new ColonAction(), ":", ':');
  vim.registerTrieChar(new YAction(d), "y", 'y');
  vim.registerTrieChar(new PAction(), "p", 'p');
  vim.registerTrieChar(new PercentAction(), "%", '%');
  vim.registerTrieChar(new IIAction(), "I", 'I');
  vim.registerTrieChar(new GAction(), "g", 'g');
  vim.registerTrieChar(new GGAction(), "G", 'G');
  vim.registerTrieChar(new ParagraphAction("{", d), "{", '{');
  vim.registerTrieChar(new ParagraphAction("}", d), "}", '}');
  vim.registerTrieChar(new BracketAction("[", d), "[", '[');
  vim.registerTrieChar(new BracketAction("]", d), "]", ']');
  vim.registerTrieChar(new ParenAction("(", d), "(", '(');
  vim.registerTrieChar(new ParenAction(")", d), ")", ')');
  vim.registerTrieChar(new QuoteAction("\"", d), "\"", '\"');
  vim.registerTrieChar(new QuoteAction("'", d), "'", '\'');
  vim.registerTrieChar(new QuoteAction("`", d), "`", '`');
  vim.registerTrieChar(new SlashAction(), "/", '/');
  vim.registerTrieChar(new RAction(), "r", 'r');
  vim.registerTrieChar(new IndentAction(), "=", '=');
  vim.registerTrieChar(new FindAction(finder, false, false), "f", 'f');
  vim.registerTrieChar(new FindAction(finder, false, true), "t", 't');
  vim.registerTrieChar(new FindAction(finder, true, false), "F", 'F');
  vim.registerTrieChar(new FindAction(finder, true, true), "T", 'T');
  vim.registerTrieChar(new SemicolonAction(finder), ";", ';');
  vim.registerTrieChar(new CommaAction(finder), ",", ',');
  vim.addRef(finder);
  for (auto &entry : state.provider.vimRemaps)
    vim.remapCharTrie(entry.first, entry.second);
}
