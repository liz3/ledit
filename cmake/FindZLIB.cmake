# FindZLIB.cmake -- redirect find_package(ZLIB) at the vendored zlib.
#
# libpng (find_package(ZLIB REQUIRED)) and FreeType (find_package(ZLIB)) both
# look zlib up through the module path.  We build zlib ourselves from
# third-party/zlib, so instead of letting the stock module search the system we
# hand out the target that add_subdirectory() already created.
#
# Handing out a target rather than a file path matters: the name and location
# of the built library depend on the configuration (Debug appends "d"), the
# generator (multi-config generators add a per-config sub-directory) and the
# compiler (MinGW produces libzlibstatic.a, MSVC zlibstatic.lib).  A hard-coded
# path can only ever be right for one of those combinations.  Using the target
# also gives the consumer the generated zconf.h include path and a proper build
# dependency, so zlib is guaranteed to be built first.
#
# This file is only picked up because the top-level CMakeLists.txt puts
# ${CMAKE_SOURCE_DIR}/cmake on CMAKE_MODULE_PATH.

if(TARGET zlibstatic)
  set(_ledit_zlib_target zlibstatic)
elseif(TARGET zlib)
  set(_ledit_zlib_target zlib)
else()
  message(FATAL_ERROR
    "find_package(ZLIB) was reached before the vendored zlib was available.\n"
    "add_subdirectory(third-party/zlib) must run before anything that looks "
    "for ZLIB.")
endif()

get_target_property(_ledit_zlib_source_dir ${_ledit_zlib_target} SOURCE_DIR)
get_target_property(_ledit_zlib_binary_dir ${_ledit_zlib_target} BINARY_DIR)

if(NOT TARGET ZLIB::ZLIB)
  add_library(ZLIB::ZLIB ALIAS ${_ledit_zlib_target})
endif()

# zlib.h ships in the source tree, zconf.h is generated into the build tree.
set(ZLIB_INCLUDE_DIR "${_ledit_zlib_source_dir}")
set(ZLIB_INCLUDE_DIRS "${_ledit_zlib_source_dir}" "${_ledit_zlib_binary_dir}")
set(ZLIB_LIBRARY ZLIB::ZLIB)
set(ZLIB_LIBRARIES ZLIB::ZLIB)

set(ZLIB_VERSION_STRING "")
if(EXISTS "${_ledit_zlib_source_dir}/zlib.h")
  file(STRINGS "${_ledit_zlib_source_dir}/zlib.h" _ledit_zlib_version_line
       REGEX "^#define[ \t]+ZLIB_VERSION[ \t]+\"[^\"]*\"$")
  if(_ledit_zlib_version_line MATCHES "\"([^\"]*)\"")
    set(ZLIB_VERSION_STRING "${CMAKE_MATCH_1}")
  endif()
  unset(_ledit_zlib_version_line)
endif()
set(ZLIB_VERSION "${ZLIB_VERSION_STRING}")
set(ZLIB_VERSION_MAJOR "")
set(ZLIB_VERSION_MINOR "")
set(ZLIB_VERSION_PATCH "")
if(ZLIB_VERSION_STRING MATCHES "^([0-9]+)\.([0-9]+)\.([0-9]+)")
  set(ZLIB_VERSION_MAJOR "${CMAKE_MATCH_1}")
  set(ZLIB_VERSION_MINOR "${CMAKE_MATCH_2}")
  set(ZLIB_VERSION_PATCH "${CMAKE_MATCH_3}")
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(ZLIB
  REQUIRED_VARS ZLIB_LIBRARY ZLIB_INCLUDE_DIR
  VERSION_VAR ZLIB_VERSION_STRING)

unset(_ledit_zlib_target)
unset(_ledit_zlib_source_dir)
unset(_ledit_zlib_binary_dir)
