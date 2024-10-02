#include "accent.h"

std::unordered_map<char, std::vector<std::string>> accented_characters = {
    {'a', {"à", "á", "â", "ä", "ã", "å", "æ"}},
    {'e', {"è", "é", "ê", "ë"}},
    {'i', {"ì", "í", "î", "ï"}},
    {'o', {"ò", "ó", "ô", "ö", "õ", "ø"}},
    {'u', {"ù", "ú", "û", "ü"}},
    {'s', {"ß", "ş", "ș", "ś", "š"}},
    {'S', {"ẞ", "Ś", "Š", "Ş", "Ș"}},
    {'c', {"ç"}},
    {'n', {"ñ"}},
    {'y', {"ý", "ÿ"}},
    {'A', {"À", "Á", "Â", "Ä", "Ã", "Å", "Æ"}},
    {'E', {"È", "É", "Ê", "Ë"}},
    {'I', {"Ì", "Í", "Î", "Ï"}},
    {'O', {"Ò", "Ó", "Ô", "Ö", "Õ", "Ø"}},
    {'U', {"Ù", "Ú", "Û", "Ü"}},
    {'C', {"Ç"}},
    {'N', {"Ñ"}},
    {'Y', {"Ý", "Ÿ"}}};

void Accent::processEvent(GLFWwindow *window, int key, int scancode, int action,
                          int mods) {
  if (state == 3)
    return;
  if (state == 2 && key == lastKey && action == GLFW_RELEASE) {
    state = 3;
    return;
  }
  if (state == 1) {
    if ((action == GLFW_RELEASE && key == lastKey) || key != lastKey) {
      state = 0;
    } else {
      std::chrono::time_point<std::chrono::system_clock> now =
          std::chrono::system_clock::now();
      auto duration = now.time_since_epoch();
      auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(duration)
                    .count();
      if (ms - start > 400) {
        provideComplete();
      }
    }
    return;
  }

  if (state != 0)
    return;
  bool ctrl_pressed =
      glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS ||
      glfwGetKey(window, GLFW_KEY_RIGHT_CONTROL) == GLFW_PRESS ||
      mods & GLFW_MOD_CONTROL;
  bool shift_pressed = glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS;
  bool alt_pressed = glfwGetKey(window, GLFW_KEY_LEFT_ALT) == GLFW_PRESS;
  if (!ctrl_pressed && !alt_pressed && action == GLFW_PRESS && key >= 65 &&
      key <= 90 && state == 0 &&
      accented_characters.count(getCharFromKey(key))) {
    lastKey = key;
    wasShift = shift_pressed;

    needInterrupt = true;
  }
}
void Accent::interrupt() {
  if (needInterrupt) {
    state = 1;
    std::chrono::time_point<std::chrono::system_clock> now =
        std::chrono::system_clock::now();
    auto duration = now.time_since_epoch();
    start =
        std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
    glfwWaitEventsTimeout(0.4);
    if (state == 1)
      provideComplete();
  }
  needInterrupt = false;
}
bool Accent::checkBlock(int key, int action) {
  if (state >= 1) {
    if (action == GLFW_PRESS &&
        (key == GLFW_KEY_ENTER || key == GLFW_KEY_ESCAPE ||
         key == GLFW_KEY_SPACE || key == GLFW_KEY_BACKSPACE)) {
      state = 0;
      if (key == GLFW_KEY_ESCAPE)
        return true;
    }
  }
  return state != 0;
}
bool Accent::shouldBlock() { return state >= 2; }
bool Accent::blockCp(uint32_t cp) {
  if (state == 3) {
    if (cp >= '0' && cp <= '9') {
      doInsert(cp - '0');
      state = 0;
      return true;
    }
    state = 0;
  }
  return state != 0;
}
Utf8String Accent::getStatus() {
  char in = getCharFromKey(lastKey);
  Utf8String content("Accents: ");
  const auto &vec = accented_characters[in];
  for (size_t i = 0; i < vec.size(); i++) {
    content += vec[i];
    content += Utf8String("(" + std::to_string(i + 1) + ")");
    if (i < vec.size() - 1)
      content += Utf8String(" | ");
  }
  return content;
}
void Accent::provideComplete() { state = 2; }
void Accent::doInsert(int num) {
  auto &entries = accented_characters[getCharFromKey(lastKey)];
  if (num - 1 >= entries.size())
    return;
  auto &string = entries[num - 1];
  cursor->removeOne();
  cursor->append(Utf8String(string));
}
char Accent::getCharFromKey(int k) {
  if (wasShift)
    return (char)k;
  return (char)k + 32;
}