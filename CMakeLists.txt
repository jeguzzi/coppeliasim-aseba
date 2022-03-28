cmake_minimum_required(VERSION 3.5)
project(simExtAseba)
set(CMAKE_C_STANDARD 99)
if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  # add_compile_options(-Wall -Wextra -Wpedantic)
endif()

if(WIN32)
  # avoid to overwrite std::max in dashel-win32.cpp
  add_compile_definitions(NOMINMAX)
  # force it to build .lib
  option(BUILD_SHARED_LIBS "" OFF)
endif()

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_MACOSX_RPATH 1)
option(ZEROCONF "" OFF)
list(APPEND CMAKE_MODULE_PATH
  ${CMAKE_CURRENT_SOURCE_DIR}/CMakeModules
)

if(NOT LIBPLUGIN_DIR)
  if(DEFINED ENV{COPPELIASIM_ROOT_DIR})
    set(LIBPLUGIN_DIR $ENV{COPPELIASIM_ROOT_DIR}/programming/libPlugin)
  else()
    set(LIBPLUGIN_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../libPlugin)
  endif()
endif()
list(APPEND CMAKE_MODULE_PATH
  ${LIBPLUGIN_DIR}/cmake
)

find_package(CoppeliaSim 4.3 REQUIRED)
set(LIB_INSTALL_DIR ${COPPELIASIM_LIBRARIES_DIR} CACHE FILEPATH "")
set(MODEL_FOLDER ${COPPELIASIM_MODELS_DIR}/robots/mobile)

macro(codesign target)
endmacro(codesign)
add_compile_options(-pedantic)

if(DEFINED ENV{DASHEL_DIR})
  set(DASHEL_DIR $ENV{DASHEL_DIR})
else()
  set(DASHEL_DIR ${PROJECT_SOURCE_DIR}/dashel)
endif()

add_subdirectory(${DASHEL_DIR} dashel)

if(DEFINED ENV{ASEBA_DIR})
  set(ASEBA_DIR $ENV{ASEBA_DIR})
else()
  set(ASEBA_DIR ${PROJECT_SOURCE_DIR}/aseba)
endif()

set(LIB_VERSION_MAJOR 3) # Must be bumped for incompatible ABI changes
set(LIB_VERSION_MINOR 0)
set(LIB_VERSION_PATCH 0)
set(LIB_VERSION_STRING ${LIB_VERSION_MAJOR}.${LIB_VERSION_MINOR}.${LIB_VERSION_PATCH})
include(FeatureSummary)
include(${ASEBA_DIR}/CMakeModules/aseba_conf.cmake)
include(${ASEBA_DIR}/CMakeModules/cpp_features.cmake)

include_directories(
  ${ASEBA_DIR}/aseba
)

if(ZEROCONF)
  message("With support for Zeroconf")
  add_compile_definitions(ZEROCONF=1)
  include(zeroconf.cmake)
  set(HAS_ZEROCONF_SUPPORT true)
  add_subdirectory(${ASEBA_DIR}/aseba/transport/dashel_plugins dashel_plugins)
  set(EXTRA_LIBS
    asebazeroconf
    asebadashelplugins
  )
endif()

add_subdirectory(${ASEBA_DIR}/aseba/common asebacommon)
add_subdirectory(${ASEBA_DIR}/aseba/transport/buffer asebavmbuffer)
add_subdirectory(${ASEBA_DIR}/aseba/vm asebavm)

set(CMAKE_C_STANDARD 99)
set(CMAKE_CXX_STANDARD 17)
if(NOT LIBPLUGIN_DIR)
  if(DEFINED ENV{COPPELIASIM_ROOT_DIR})
    set(LIBPLUGIN_DIR $ENV{COPPELIASIM_ROOT_DIR}/programming/libPlugin)
  else()
    set(LIBPLUGIN_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../libPlugin)
  endif()
endif()
list(APPEND CMAKE_MODULE_PATH
  ${LIBPLUGIN_DIR}/cmake
)

find_package(CoppeliaSim 4.3 REQUIRED)
find_package(OpenCV 4 REQUIRED )

include_directories(${OpenCV_INCLUDE_DIRS} )
include_directories(${CMAKE_CURRENT_BINARY_DIR}/generated)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/external)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

coppeliasim_generate_stubs(
  ${CMAKE_CURRENT_BINARY_DIR}/generated
  XML_FILE ${CMAKE_CURRENT_SOURCE_DIR}/callbacks.xml
  LUA_FILE ${CMAKE_CURRENT_SOURCE_DIR}/simAseba.lua
)

# add_custom_target(
#  generate_thymio_lua ALL
#  COMMAND python3 ${CMAKE_CURRENT_SOURCE_DIR}/generate_thymio_lua.py ${CMAKE_CURRENT_BINARY_DIR}/generated
#  BYPRODUCTS ${CMAKE_CURRENT_BINARY_DIR}/generated/simThymio-typecheck.lua
#  COMMENT "Generating Thymio lua script"
# )

add_compile_definitions(TEXTURE_DIR="${COPPELIASIM_MODELS_DIR}")

configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/config.h.in
  ${CMAKE_CURRENT_BINARY_DIR}/config.h
ESCAPE_QUOTES)

coppeliasim_add_plugin(simExtAseba
  SOURCES
  src/plugin.cpp
  src/coppeliasim_thymio2.cpp
  src/aseba_node.cpp
  src/aseba_thymio2_descriptions.c
  src/aseba_thymio2_natives.cpp
  src/aseba_thymio2.cpp
  src/aseba_default_description.c
  src/aseba_network.cpp
)

find_package(Python3 REQUIRED COMPONENTS Interpreter)
add_custom_target(
  generate_thymio_lua ALL
  COMMAND
    export PYTHONPATH=$PYTHONPATH:${LIBPLUGIN_DIR}/simStubsGen
    &&
    ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/helpers/generate_split_reference.py --directory ${CMAKE_CURRENT_BINARY_DIR}/generated
    &&
    ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/helpers/generate_markdown_api.py --directory ${CMAKE_CURRENT_BINARY_DIR}/generated --file reference_Aseba.xml
    &&
    ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/helpers/generate_markdown_api.py --directory ${CMAKE_CURRENT_BINARY_DIR}/generated --file reference_Thymio.xml
    &&
    ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/helpers/generate_lua.py --directory ${CMAKE_CURRENT_BINARY_DIR}/generated
  BYPRODUCTS ${CMAKE_CURRENT_BINARY_DIR}/generated/simThymio-typecheck.lua
  DEPENDS simExtAseba
  COMMENT "Generating docs and Thymio lua script"
)

target_link_libraries(simExtAseba ${OpenCV_LIBS} dashel asebacommon asebavmbuffer asebavm
  ${EXTRA_LIBS}
)

if(DEFINED MODEL_FOLDER)
  install(
    FILES
      ${CMAKE_CURRENT_SOURCE_DIR}/models/thymio.ttm
      ${CMAKE_CURRENT_SOURCE_DIR}/models/thymio-body-diffusionMap0.png
      ${CMAKE_CURRENT_SOURCE_DIR}/models/thymio-body-diffusionMap1.png
      ${CMAKE_CURRENT_SOURCE_DIR}/models/thymio-body-diffusionMap2.png
      ${CMAKE_CURRENT_SOURCE_DIR}/models/thymio-body-texture.png
    DESTINATION ${MODEL_FOLDER}
  )
endif()

install(
  FILES
    ${CMAKE_CURRENT_BINARY_DIR}/generated/simThymio-typecheck.lua
    ${CMAKE_CURRENT_SOURCE_DIR}/simThymio.lua
  DESTINATION ${COPPELIASIM_LUA_DIR}
)
