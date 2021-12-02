# Ledit
Very simple GPU Rendered text editor without any bullshit.  
With keybinds inspired by emacs.

## Motivation
The base motivation was just simply that i wanted to have a look into OpenGL and doing GPU accelerated things, i did not plan to create a text editor from the get go.  
After starting to experiment a bit i decided to call this a small side Project and implement a proper editor.

![image](https://github.com/liz3/ledit/blob/master/assets/screenshot.png?raw=true)
![image](https://github.com/liz3/ledit/blob/master/assets/screenshot2.png?raw=true)

## Building
To build ledit you only need [CMake](https://cmake.org/) and a C/C++ compiler which supports C++17.
ledit requires GLFW and freetype2 but these are included in the [third-party](/third-party) folder.

### GNU/Linux & MacOS
To get the prerequisites run:
```sh
./init.sh
```
Then you can build the executable with
```
./build_release.sh
```
For debug builds use `build_debug.sh`

### Windows
You will need at least a recent version of the windows MSVC C++ compiler, its better to install entire visual studio for the sake of the case that vs installer might leave out needed components.
then run in a CMD in the folder:
```
git submodule update --init
mkdir Release
cd Release
cmake ..
cmake --build . --config Release
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
- fira_code.h - inlined version of the fira code font.
- highlighting.h - simple highlighting engine
- languages.h - contains language profiles for auto complete
- third-party - ledit dependencies
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
C-shift-p - move current line up
C-shift-n - move current line down
# more soon!!!

Operations:
C-x-s - Save to last path, if no path present, ledit will ask for a path.
C-x-n - Save to new location, note that this will not overwrite the default save path, to overwrite the default path, save then load.
C-x-o - Load new file, this will replace the current file, non existing files will still load but be marked as New Files.
C-x-k - switch between files that where opened in the session, note that this will not preserve edits in the current file, save these first,
Note: pressing this again will rotate through files that where open.

C-z - Undo.
M-w/C-c - Copy
C-y/C-v - Paste

C-space - Toggles selection selection on and off.

Misc:
C-x-l - Toggle Line numbers.
C-+   - Increase font size
C--(-)- decrease font size



```
