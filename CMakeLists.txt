cmake_minimum_required(VERSION 3.16)
project("ledit")
set(CMAKE_CXX_STANDARD 17)

# cmake/FindZLIB.cmake and cmake/FindPNG.cmake redirect find_package(ZLIB) and
# find_package(PNG) at the copies we build from third-party/.  libpng and
# FreeType both call find_package() internally, so this has to be on the module
# path before either of them is added.
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(DISABLE_PACKAGE_CONFIG_AND_INSTALL ON)
set(SKIP_INSTALL_ALL ON)
# We link the static libpng, so don't spend time on the shared library, the
# tools or the test suite.  CACHE/FORCE because these are option() in libpng.
set(PNG_TOOLS OFF CACHE BOOL "" FORCE)
set(PNG_TESTS OFF CACHE BOOL "" FORCE)
set(PNG_SHARED OFF CACHE BOOL "" FORCE)
set(PNG_STATIC ON CACHE BOOL "" FORCE)
set(ZLIB_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(LEDIT_SOURCES src/glad.c
 src/la.cc 
 src/accent.cc
  src/cursor.cc 
  src/font_atlas.cc 
  src/highlighting.cc
   src/providers.cc 
   src/selection.cc 
   src/shader.cc 
   src/state.cc 
   src/utf8String.cc 
   src/utils.cc 
   src/vim.cc 
   src/main.cc
    src/vim_actions/EscapeAction.cc
    src/vim_actions/register.cc
src/vim_actions/BackspaceAction.cc
src/vim_actions/EnterAction.cc
src/vim_actions/TabAction.cc
src/vim_actions/IAction.cc
src/vim_actions/HAction.cc
src/vim_actions/JAction.cc
src/vim_actions/KAction.cc
src/vim_actions/LAction.cc
src/vim_actions/AAction.cc
src/vim_actions/AAAction.cc
src/vim_actions/WAction.cc
src/vim_actions/EAction.cc
src/vim_actions/BAction.cc
src/vim_actions/OAction.cc
src/vim_actions/OOAction.cc
src/vim_actions/DAction.cc
src/vim_actions/UAction.cc
src/vim_actions/VAction.cc
src/vim_actions/CAction.cc
src/vim_actions/DollarAction.cc
src/vim_actions/ZeroAction.cc
src/vim_actions/ColonAction.cc
src/vim_actions/YAction.cc
src/vim_actions/PAction.cc
src/vim_actions/PercentAction.cc
src/vim_actions/IIAction.cc
src/vim_actions/GGAction.cc
src/vim_actions/GAction.cc
src/vim_actions/ParagraphAction.cc
src/vim_actions/BracketAction.cc
src/vim_actions/ParenAction.cc
src/vim_actions/QuoteAction.cc
src/vim_actions/SlashAction.cc
src/vim_actions/FontSizeAction.cc
src/vim_actions/MoveAction.cc
src/vim_actions/RAction.cc
src/vim_actions/Finder.cc
src/vim_actions/FindAction.cc
src/vim_actions/SemicolonAction.cc
src/vim_actions/CommaAction.cc
src/vim_actions/DDAction.cc
src/vim_actions/XAction.cc
src/vim_actions/CCAction.cc
src/vim_actions/CtrlUAction.cc
src/vim_actions/CommentAction.cc
src/vim_actions/SimpleCopy.cc
src/vim_actions/IndentAction.cc)
add_executable(ledit ${LEDIT_SOURCES})

if(WIN32)
  # On Windows a Release build is a GUI-subsystem program: no console window, a
  # WinMain entry point and the application icon.  All of it is expressed with
  # per-config generator expressions rather than an if() on CMAKE_BUILD_TYPE,
  # because that variable is empty for multi-config generators -- under Visual
  # Studio the old if() never fired and `--build --config Release` produced a
  # console executable without an icon.
  set_target_properties(ledit PROPERTIES WIN32_EXECUTABLE $<CONFIG:Release>)
  target_compile_definitions(ledit PRIVATE $<$<CONFIG:Release>:LEDIT_WIN_MAIN>)
  target_sources(ledit PRIVATE
    $<$<CONFIG:Release>:${CMAKE_CURRENT_SOURCE_DIR}/assets/appicon.rc>)
  # -mwindows is a GCC driver flag and only means anything to MinGW; MSVC takes
  # the subsystem from WIN32_EXECUTABLE above.  It also has to be a link option
  # on this target only: it used to be assigned over the whole of
  # CMAKE_C_FLAGS_RELEASE before the third-party subdirectories were added,
  # which dropped /O2 and /DNDEBUG from zlib, libpng, FreeType and GLFW.
  if(MINGW)
    target_link_options(ledit PRIVATE $<$<CONFIG:Release>:-mwindows>)
  endif()
endif()
# Order matters: each subproject looks up the ones before it through
# find_package(), which cmake/Find{ZLIB,PNG}.cmake answer with the targets
# defined here.  Nothing needs to spell out library file names or include
# directories -- the targets carry both, for every configuration and generator.
add_subdirectory(third-party/zlib)
set(PNG_FRAMEWORK OFF CACHE BOOL "" FORCE)
add_subdirectory(third-party/lpng1639)
if(UNIX AND NOT APPLE)
    if(GLFW_USE_WAYLAND OR GLFW_BUILD_WAYLAND)
        set(GLFW_BUILD_X11 OFF)
        set(GLFW_BUILD_WAYLAND ON)
    else()
        set(GLFW_BUILD_X11 ON)
        set(GLFW_BUILD_WAYLAND OFF)
    endif()
endif()
add_subdirectory(third-party/glfw)

# FT_WITH_PNG was renamed in FreeType 2.11; the current spelling is
# FT_REQUIRE_PNG/FT_REQUIRE_ZLIB.  Requiring them turns a broken lookup into a
# configure error instead of silently building a FreeType without colour-bitmap
# support.
set(FT_REQUIRE_PNG ON CACHE BOOL "" FORCE)
set(FT_REQUIRE_ZLIB ON CACHE BOOL "" FORCE)

add_subdirectory(third-party/freetype2)
if (NOT WIN32 AND NOT APPLE)
    target_link_libraries(ledit PUBLIC fontconfig dl)
endif()
target_link_libraries(ledit PRIVATE glfw freetype)
 
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_definitions(-DLEDIT_DEBUG)
 endif()
if(APPLE OR UNIX)
    if(CMAKE_BUILD_TYPE STREQUAL "Release")
        set(CMAKE_CXX_FLAGS_RELEASE "-O2")
    endif()
endif()