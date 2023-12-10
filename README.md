# Ledit
Very simple GPU Rendered text editor without Bloat.  
With keybinds inspired by emacs and a(partial) vim mode.  
Ledit runs on all 3 major operating systems: GNU/linux, macOS and Windows.
if you just want to test the editor then [download it from the releases](https://github.com/liz3/ledit/releases/latest)

## Motivation
The base motivation was just simply that i wanted to have a look into OpenGL and doing GPU accelerated things, i did not plan to create a text editor from the get go.  
After starting to experiment a bit i decided to call this a small side Project and implement a proper editor.

**Edit:** When i started this project it was really just a simple simple text editor, but ive added features like commands, the vim mode and so on since i use this editor on a daily basis and needed them. I try to keep it as simple as possible, while respecting what i need of it.



![image](https://github.com/liz3/ledit/blob/master/assets/screenshot.png?raw=true)
![image](https://github.com/liz3/ledit/blob/master/assets/screenshot2.png?raw=true)
![image](https://github.com/liz3/ledit/blob/master/assets/screenshot3.png?raw=true)

## Encodings
Ledit supports the entire utf-8 range.

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
Ledit builds with MSVC and does not require a unix betewen layer like Cgywin.
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
- src/main.cc: main rendering logic and keyboard callbacks.
- src/state.h: logic for controlling and state point.
- src/cursor.h: this is the most important file besides main, it manages the text state, what to render and where. and implements all logic components for manipulation.
- src/shader.h: manages shader loading.
- src/font_atlas.h: font atlas and width calculation.
- src/shaders.h: inlined shaders.
- src/highlighting.h: simple highlighting engine.
- src/languages.h: contains modes for certain languages for highlighting.
- src/provider.h: This contains the config parser and providers for folder autocomplete and other related things.
- src/selection.h: Small structure to keep track of selection state.
- src/la.(cc/h): Vectors implementation for coords and RGBA colors.
- src/utf8String.h: Utf8 string implementation.
- src/vim.h: Vim state management.
- src/vim_actions.h: Implementation of all the vim motions.
- third-party: ledit dependencies.
```
There are more but these are self explaining.
## Config
ledit can have a config in your home directory `~/.ledit/config.json`.  
The following values can be set(without the comments)  
For the colors there are default values, for the font face either remove it completely or make sure its a valid path.
```jsonc
{
  "use_spaces": true, // when pressing tab insert spaces equal to the tab_width, this is true by default.
  "tab_width": 2, // This controls how many spaces the \t character is wide, further it controls the amount of spaces used in case "use_spaces" is true,
  "auto_reload": false, // if a file changed on disk and the editor asks the OS, automatically reload buffer content
  "save_before_command": false, // save_buffer before running command
  "vim_mode": false, // vim mode, see vim section
  "colors": {
    "comment_color": [
      127, 127, 127, 127 // Comment color if a active mode is present, in RGBA (0-255)
    ],
    "default_color": [
      242, 242, 242, 242 // Default color for text, in RGBA (0-255)
    ],
    "keyword_color": [
      153, 25, 51, 255 // Keyword color if a active mode is present, in RGBA (0-255)
    ],
    "special_color": [
      51, 51, 204, 255 // Special words color if a active mode is present, in RGBA (0-255)
    ],
    "string_color": [
      51, 153, 102, 255 // String literak color if a active mode is present, in RGBA (0-255)
    ],
   "background_color": [
     0, 0, 0, 255   // Editor background color RGBA (0-255)
    ],
   "selection_color": [
     0, 0, 0, 255   // Selection area color RGBA (0-255)
    ],
   "highlight_color": [
     0, 0, 0, 255   // Color of the active line background highlight. RGBA (0-255)
    ],
   "number_color": [
     0, 0, 160, 255   // Color used for numbers. RGBA (0-255)
    ],
   "line_number_color": [
     0, 0, 160, 255   // Color used for line numbers RGBA (0-255)
    ],
   "status_color": [
     0, 0, 160, 255   // Color used for the status line, buffer information RGBA (0-255)
    ],
   "minibuffer_color": [
     0, 0, 160, 255   // Color used for the Minibuffer, actions like search, replace, save new and so on. RGBA (0-255)
    ],
   "cursor_color": [
     0, 0, 160, 255   // Color used for the default cursor RGBA (0-255)
    ],
   "vim_cursor_color": [
     0, 0, 160, 255   // Color used for the vim cursor(NORMAL/VISUAL mode) RGBA (0-255)
    ]
  },
  // optional load additional font files
  "extra_fonts": [
  ],
  "window_transparency": true, // if the window is allowed to be transparent
  "font_face": "/Users/liz3/Library/Fonts/FiraCode-Regular.ttf", // TTF font face path
  "commands": {
      // see the command section
  }
}
```
## Highlighting for extra languages
**Note: There are a few examples in [language-syntaxes](/language-syntaxes), further since this folder takes priority, it is possible to overwrite existing build in modes too by specifying the extensions, ledit will first use the folder and only if theres no match use its built in langauge support.**

ledit can load support for highlighting extra languages via the folder `~/.ledit/languages/`.
A file in languages supports the properties shown in the following CMake example(note that this is not complete)

`~/.ledit/languages/cmake.json`
```json
  {
    "mode_name": "CMake",
    "key_words": [
      "if",
      "else",
      "endif",
      "BOOL",
      "AND",
      "OR",
      "ON",
      "OFF",
      "GLOB",
      "FORCE",
      "CACHE"
    ],
    "special_words": [
      "set",
      "include_directories",
      "add_library",
      "add_executable",
      "target_include_directories",
      "target_link_libraries",
      "project",
      "add_subdirectory",
      "cmake_minimum_required",
      "link_libraries",
      "set_target_properties",
      "execute_process",
      "add_definitions",
      "string",
      "message"
    ],
    "single_line_comment": "#",
    "multi_line_comment": ["#[[", "]]"],
    "escape_character": "\\",
    "file_extensions": [
      "CMakeLists.txt",
      "cmake"
    ],
    "string_characters": "\""
  }
```

### Commands
The config can contain the `commands` json object which can look like this:
```json
"command-name": "shell command here",
```
Then using `C-:` you can invoke the command name which will run the respective shell command.

The shell command can contain certain Placeholders which will then be replaced at runtime:
* `$file_path` - This will will be replaced with the relative path to the file path of the active buffer, relative to the cwd of the current ledit instance.
* `$file_name` - File basename, or empty string.
* `$file_extension` - File extension, or empty string.
* `$file_basename` - File basename without extension, or empty string.
* `$cwd` - Absolute path of ledits working directory.
* `$selection_content` - if active the content of the current selection, note that the selection can contain new line characters, if not active this is a empty string `""`.

### Vim mode
Ledit does support a limited set of vim motions using the `vim_mode`, this is because ive been using vim motions and wanted my editor to support it.
Enabling the vim mode drastically changes how the key mapping works, the emacs like keybindings are hardcoded in the main.cc file, since vim motions are a lot more complex then that,
they are seperated and you can actually edit them.

There are a lot of unspported options but most of the basics do work.

#### Commands:
The way the command input works is most cases is that you use something like `:e` and press enter, the editor will then switch into the existing implementation for the emacs mode and prompt again,
i am aware this isn't entirely convinient but it was a loooot less work to implement and it was already a lot.
There are some commands where this is supported though,

```
:b - switch buffer
:c - run a command
:c <command> - run a command directly
:ck - kill running command
:co - open last command buffer
:%s - Start replace
:lw - toggle line wrapping(experimental)
:hl - toggle highlighting of the active line
:ln - toggle line numbers
:font - switch main font
:config - open ledits config in a buffer
:mode - switch language mode
:mode <extension> - switch to a mode given the extension like js/cpp/sh...
:e - open new file
:e <path> - directly open new path
:n - create new empty buffer
:bd - delete active buffer
:w/q/a/! - exit actions

```
#### key remaps 
You can create `~/.ledit/vim_keys.json` which has a mapping of character key => action.
That means a config like
```json
{

  "x": "d",
  "d": "x"
}
```
Switches key x to trigger delete and d to trigger x.


## Info
Here are some infos.
### Standard input & output
Ledit can work with stdin/out by passing `-` as file name, **NOTE**: saving will print once then exit!
### Keybinds for default mode
C stands for CTRL, M for alt/meta.
```
ESC:
Escape serves two purposes and behaves a bit like VIM.
If you are currently in a minibuffer(command action) that cancels that action, escape again closes the ledit instance.
Otherwise this instantly closes the instance.

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
C-%   - Jump to matching {[()]}

Search:
C-s will prompt for input and with enter its then possible to search that term case sensitive!

Manipulation:

C-d - delete character after the cursor
M-d - delete the next word including a whitespace.
C-shift-p - move current line up
C-shift-n - move current line down
C-w - cut selection
C-r - replace, first asks for search then for replace\, use SHFT-RET to replace all matches.  
C-x-/ - If a mode is active either comment or uncomment the cursor line or the selected lines, does not work for raw text mode.

Operations:
C-:   - Run command
C-x-: - See output from last command
C-x-p - If theres a running command, kill it
C-x-s - Save to last path, if no path present, ledit will ask for a path.
C-x-n - Save to new location, note that this will not overwrite the default save path, to overwrite the default path, save then load.
C-x-o - Load new file, this will replace the current file, non existing files will still load but be marked as New Files.
C-x-k - switch between open files(buffers) in the session,
Note: pressing this again will rotate through files that where open.
C-TAB - fast rotate through open files(buffers).

C-z - Undo.
M-w/C-c - Copy
C-y/C-v - Paste

C-space - Toggles selection mode on and off.

Misc:
C-x-l - Toggle Line numbers.
C-x-r - Toggle line wrapping(this is still half broken)
C-+   - Increase font size
C--(-)- decrease font size
C-x-0 - Load new font file, note that doing this will persist it in the config.
C-x-h - Toggle highlighting of the active line.
C-x-m - Switch active mode for current buffer.
C-x-w - close current buffer if its not the only one, otherwise use ESC.

```
# LICENSE
Ledit is free software following [GPL 2.0](/LICENSE).
