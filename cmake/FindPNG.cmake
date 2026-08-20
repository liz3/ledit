# FindPNG.cmake -- redirect find_package(PNG) at the vendored libpng.
#
# FreeType calls find_package(PNG) to decide whether it can decode PNG-compressed
# embedded bitmaps (colour emoji).  We build libpng ourselves from
# third-party/lpng1639, so we hand FreeType the target that add_subdirectory()
# created instead of letting the stock module search the system.
#
# The stock FindPNG only accepts a file path in PNG_LIBRARY: it builds an
# UNKNOWN IMPORTED target and guards the IMPORTED_LOCATION with if(EXISTS ...).
# Pre-seeding PNG_LIBRARY with the target name "png_static", as this project
# used to do, therefore produces a PNG::PNG target with no location at all and
# CMake fails at generate time with "IMPORTED_LOCATION not set for imported
# target PNG::PNG".  Aliasing the real target avoids that entirely and carries
# the include directories, the zlib dependency and the build ordering with it.
#
# This file is only picked up because the top-level CMakeLists.txt puts
# ${CMAKE_SOURCE_DIR}/cmake on CMAKE_MODULE_PATH.

if(TARGET png_static)
  set(_ledit_png_target png_static)
elseif(TARGET png_shared)
  set(_ledit_png_target png_shared)
else()
  message(FATAL_ERROR
    "find_package(PNG) was reached before the vendored libpng was available.\n"
    "add_subdirectory(third-party/lpng1639) must run before anything that "
    "looks for PNG, and at least one of PNG_STATIC/PNG_SHARED must be ON.")
endif()

get_target_property(_ledit_png_source_dir ${_ledit_png_target} SOURCE_DIR)
get_target_property(_ledit_png_binary_dir ${_ledit_png_target} BINARY_DIR)

if(NOT TARGET PNG::PNG)
  add_library(PNG::PNG ALIAS ${_ledit_png_target})
endif()

# png.h ships in the source tree, pnglibconf.h is generated into the build tree.
set(PNG_PNG_INCLUDE_DIR "${_ledit_png_source_dir}")
set(PNG_INCLUDE_DIR "${_ledit_png_source_dir}" "${_ledit_png_binary_dir}")
set(PNG_INCLUDE_DIRS "${_ledit_png_source_dir}" "${_ledit_png_binary_dir}")
set(PNG_LIBRARY PNG::PNG)
set(PNG_LIBRARIES PNG::PNG)
set(PNG_DEFINITIONS "")

set(PNG_VERSION_STRING "")
if(EXISTS "${_ledit_png_source_dir}/png.h")
  file(STRINGS "${_ledit_png_source_dir}/png.h" _ledit_png_version_line
       REGEX "^#define[ \t]+PNG_LIBPNG_VER_STRING[ \t]+\"[^\"]*\"$")
  if(_ledit_png_version_line MATCHES "\"([^\"]*)\"")
    set(PNG_VERSION_STRING "${CMAKE_MATCH_1}")
  endif()
  unset(_ledit_png_version_line)
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(PNG
  REQUIRED_VARS PNG_LIBRARY PNG_PNG_INCLUDE_DIR
  VERSION_VAR PNG_VERSION_STRING)

unset(_ledit_png_target)
unset(_ledit_png_source_dir)
unset(_ledit_png_binary_dir)
