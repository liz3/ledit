#ifndef LEDIT_WINDOWS_H
#define LEDIT_WINDOWS_H

#include "state.h"
#include "../third-party/glfw/include/GLFW/glfw3.h"
#include "shader.h"
#include <unordered_map>
#include "fcxit5/ibus.h"

struct MidState {
    float WIDTH = 0, HEIGHT = 0;
    int maxRenderWidth = 0, fontSize = 0;
};

struct Window {
    GLFWwindow* window = nullptr;
    State* state = nullptr;
    FontAtlas* fontAtlas = nullptr;
    std::map<std::string, Shader*> shaders;
    MidState midState;
    LeditIBus fcxitBus;
    ~Window(){
        if(window) {
            glfwDestroyWindow(window);
            window = nullptr;
        }
        if(state){
            state->freeVim();
            delete state;
            state = nullptr;
        }
        if(fontAtlas) {
            delete fontAtlas;
            fontAtlas = nullptr;
        }
        for(auto entry : shaders){
            entry.second->removeShader();
            delete entry.second;
        }
    }
};
class WindowManager {
public:
    std::unordered_map<GLFWwindow*, Window*> windows;
    Window* active_window = nullptr;
    Window* pending_new = nullptr;
    void addAndActivate(Window* window) {
        windows[window->window] = window;
        activateWindow(window->window);
    };
    void removeWindow(GLFWwindow* win){
        if(active_window && active_window->window == win)
            active_window = nullptr;
        windows.erase(win);
    }
    void activateWindow(GLFWwindow* window) {
        active_window = windows[window];
        if(active_window->state->provider.allowTransparency){
             glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, 1);
        } else {
            glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, 0);
        }
    }

};

#endif