# Ledit
Very simple GPU Rendered text editor without any bullshit.  
With keybinds inspired by emacs.

## Motivation
The base motivation was just simply that i wanted to have a look into OpenGL and doing GPU accelerated things, i did not plan to create a text editor from the get go.  
After starting to experiment a bit i decided to call this a small side Project and implement a proper editor.

![image](https://github.com/liz3/ledit/blob/master/assets/screenshot.png?raw=true)

## Building
Right now i only tested on macOS but i will try to build this on all GNU/linux & windows.  
ledit has some dependencies you will need to build it:
* GLFW3: used to create the window.
* freetype2: required for font rendering.

(Homebrew) To get the required dependencies, do:
```
brew install glfw freetype2 glm
```

You will also need CMake.

To build the binary on mac:
```sh
mkdir build
cd build
cmake ..
make
./ledit [<some file>]
```

## Structure
```
main.cc - main rendering logic and keyboard callbacks.
state.h - logic for controlling and state point.
cursor.h - this is the most important file besides main, it manages the text state,
what to render and where. and implements all logic components for manipulation.
- shader.h - manages shader loading.
- font_atlas.h - font atlas and width calculation.
- shaders.h, inlined shaders.
fira_code.h, inlined version of the fira code font.
```

## Keybinds
C stands for CTRL, M for alt/meta.
```
ESC:
Escape serves two purposes and behaves a bit like VIM.
If you are currently in a minibuffer(command action) that cancels that action, escape again closes the ledit instance.

Navigation:
C-a - jump to line start.
C-e - jump to line end.
C-f - advance one character.
C-b - reverse one character.
C-p - go up one line.
C-n - go down one line
M-f - jump forward by one word.
M-b - jump backwards by one word.

More generalised Navigation:
C-x-a - jump to file start.
C-x-e - jump to to the last line in the file.
C-x-g - asks for a line number to jump to.

Search:
C-s will prompt for input and with enter its then possible to search that term case sensitive!

Manipulation:

C-d - delete character after the cursor
M-d - delete the next word including a whitespace.
# more soon!!!

Operations:
C-x-s - Save to last path, if no path present, ledit will ask for a path.
C-x-n - Save to new location, note that this will not overwrite the default save path, to overwrite the default path, save then load.
C-x-o - Load new file, this will replace the current file.
C-x-k - switch between files that where opened in the session, note that this will not preserve edits in the current file, save these first,
Note: pressing this again will rotate through files that where open.

C-z - Undo.

```
