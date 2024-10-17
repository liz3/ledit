cmake_minimum_required(VERSION 3.16)
project("ledit")
set(CMAKE_CXX_STANDARD 17)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(DISABLE_PACKAGE_CONFIG_AND_INSTALL ON)
set(SKIP_INSTALL_ALL ON)
set(PNG_EXECUTABLES OFF)
set(PNG_SHARED OFF)
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
if(CMAKE_BUILD_TYPE STREQUAL "Release" AND WIN32)
  set(CMAKE_C_FLAGS_RELEASE "-mwindows")
  add_definitions(-DLEDIT_WIN_MAIN)
  add_executable(ledit WIN32 ${LEDIT_SOURCES})
  target_sources(ledit PRIVATE assets/appicon.rc)
else()
    add_executable(ledit ${LEDIT_SOURCES})
endif()
add_subdirectory(third-party/zlib)
set(ZLIB_LIBRARY zlibstatic)
set(ZLIB_LIBRARIES zlibstatic)
set(ZLIB_INCLUDE_DIRS  ${CMAKE_CURRENT_SOURCE_DIR}/third-party/zlib ${CMAKE_CURRENT_BINARY_DIR}/third-party/zlib)
set(ZLIB_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/third-party/zlib ${CMAKE_CURRENT_BINARY_DIR}/third-party/zlib)
add_subdirectory(third-party/lpng1639)
set(PNG_LIBRARIES png_static)
set(PNG_LIBRARY png_static)
set(PNG_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/third-party/lpng1639 ${CMAKE_CURRENT_BINARY_DIR}/third-party/lpng1639/)
set(PNG_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/third-party/lpng1639 ${CMAKE_CURRENT_BINARY_DIR}/third-party/lpng1639/)
set(PNG_PNG_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/third-party/lpng1639 ${CMAKE_CURRENT_BINARY_DIR}/third-party/lpng1639/)
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

set(FT_WITH_PNG ON)

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