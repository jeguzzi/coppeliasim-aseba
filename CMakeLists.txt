cmake_minimum_required(VERSION 3.10)
project(simExtAseba)

set(CMAKE_C_STANDARD 99)
if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  # add_compile_options(-Wall -Wextra -Wpedantic)
  add_compile_options(-Wno-c99-extensions -Wno-gnu-flexible-array-initializer)
  # -Wc99-extensions
endif()

if(WIN32)
  # avoid to overwrite std::max in dashel-win32.cpp
  add_compile_definitions(NOMINMAX)
  # force it to build .lib
  option(BUILD_SHARED_LIBS "" OFF)
  add_definitions(-DDISABLE_WEAK_CALLBACKS)
endif()

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_MACOSX_RPATH 1)
# option(ZEROCONF "" OFF)
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/CMakeModules)

if(NOT COPPELIASIM_INCLUDE_DIR)
  if(DEFINED ENV{COPPELIASIM_ROOT_DIR})
    set(COPPELIASIM_INCLUDE_DIR $ENV{COPPELIASIM_ROOT_DIR}/programming/include)
  else()
    set(COPPELIASIM_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../include)
  endif()
endif()
if(NOT LIBPLUGIN_DIR)
  if(DEFINED ENV{COPPELIASIM_ROOT_DIR})
    set(LIBPLUGIN_DIR $ENV{COPPELIASIM_ROOT_DIR}/programming/libPlugin)
  else()
    set(LIBPLUGIN_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../libPlugin)
  endif()
endif()

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules
     ${COPPELIASIM_INCLUDE_DIR}/cmake ${LIBPLUGIN_DIR}/cmake)

macro(codesign target)

endmacro(codesign)
add_compile_options(-pedantic)

if(DEFINED ENV{DASHEL_DIR})
  set(DASHEL_DIR $ENV{DASHEL_DIR})
else()
  set(DASHEL_DIR ${PROJECT_SOURCE_DIR}/dashel)
endif()

add_subdirectory(${DASHEL_DIR} dashel EXCLUDE_FROM_ALL)

if(DEFINED ENV{ASEBA_DIR})
  set(ASEBA_DIR $ENV{ASEBA_DIR})
else()
  set(ASEBA_DIR ${PROJECT_SOURCE_DIR}/aseba)
endif()

set(LIB_VERSION_MAJOR 3) # Must be bumped for incompatible ABI changes
set(LIB_VERSION_MINOR 0)
set(LIB_VERSION_PATCH 0)
set(LIB_VERSION_STRING
    ${LIB_VERSION_MAJOR}.${LIB_VERSION_MINOR}.${LIB_VERSION_PATCH})
include(FeatureSummary)
include(${ASEBA_DIR}/CMakeModules/aseba_conf.cmake)
include(${ASEBA_DIR}/CMakeModules/cpp_features.cmake)

include_directories(${ASEBA_DIR}/aseba)

include(${CMAKE_CURRENT_SOURCE_DIR}/CMakeModules/zeroconf.cmake)
if(HAS_ZEROCONF_SUPPORT)
  message("With support for Zeroconf")
  add_compile_definitions(ZEROCONF=1)
  add_subdirectory(${ASEBA_DIR}/aseba/transport/dashel_plugins dashel_plugins
                   EXCLUDE_FROM_ALL)
  set(EXTRA_LIBS asebazeroconf asebadashelplugins)
endif()

find_package(LibXml2 REQUIRED)
if(LibXml2_FOUND)
  # add_compile_definitions(COMPILE_XML=1)
else()
  message("LibXml2 not found: won't be able to load aseba scripts from files")
endif()

add_subdirectory(${ASEBA_DIR}/aseba/common asebacommon EXCLUDE_FROM_ALL)
add_subdirectory(${ASEBA_DIR}/aseba/transport/buffer asebavmbuffer
                 EXCLUDE_FROM_ALL)
add_subdirectory(${ASEBA_DIR}/aseba/vm asebavm EXCLUDE_FROM_ALL)
add_subdirectory(${ASEBA_DIR}/aseba/compiler asebacompiler EXCLUDE_FROM_ALL)

set(CMAKE_C_STANDARD 99)
set(CMAKE_CXX_STANDARD 17)

find_package(CoppeliaSim 4.3 REQUIRED)

set(LIB_INSTALL_DIR
    ${COPPELIASIM_LIBRARIES_DIR}
    CACHE FILEPATH "")
set(MODEL_DIR ${COPPELIASIM_MODELS_DIR}/robots/mobile)

find_package(
  OpenCV 4
  COMPONENTS core imgproc imgcodecs
  REQUIRED)

include_directories(${OpenCV_INCLUDE_DIRS})
include_directories(${LIBXML2_INCLUDE_DIR})
include_directories(${CMAKE_CURRENT_BINARY_DIR}/generated)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/external)

if(${COPPELIASIM_VERSION} VERSION_GREATER_EQUAL 4.6)
  set(_PLUGIN_NAME simAseba)
  set(_PLUGIN_NAME_XML simAseba)
  set(PREFIX sim)
  set(_KEYWORD PRIVATE)
else()
  set(_PLUGIN_NAME simExtAseba)
  set(_PLUGIN_NAME_XML Aseba)
  set(PREFIX "''")
endif()

if(${COPPELIASIM_VERSION} VERSION_GREATER_EQUAL 4.8)
  set(MODEL_VERSION "v3")
elseif(${COPPELIASIM_VERSION} VERSION_GREATER_EQUAL 4.5)
  set(MODEL_VERSION "v2")
else()
  set(MODEL_VERSION "v1")
endif()

execute_process(
  COMMAND
    bash -c
    "sed s/PLUGIN_NAME/${_PLUGIN_NAME_XML}/g ${CMAKE_CURRENT_SOURCE_DIR}/lua/callbacks.xml > callbacks.xml"
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

coppeliasim_generate_stubs(
  ${CMAKE_CURRENT_BINARY_DIR}/generated XML_FILE
  ${CMAKE_CURRENT_BINARY_DIR}/callbacks.xml LUA_FILE
  ${CMAKE_CURRENT_SOURCE_DIR}/lua/simAseba.lua)

# add_custom_target( generate_thymio_lua ALL COMMAND python3
# ${CMAKE_CURRENT_SOURCE_DIR}/generate_thymio_lua.py
# ${CMAKE_CURRENT_BINARY_DIR}/generated BYPRODUCTS
# ${CMAKE_CURRENT_BINARY_DIR}/generated/simThymio-typecheck.lua COMMENT
# "Generating Thymio lua script" )

add_compile_definitions(TEXTURE_DIR="${MODEL_DIR}")
add_compile_definitions(PLUGIN_NAME_XML="${_PLUGIN_NAME_XML}")

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/include/config.h.in
               ${CMAKE_CURRENT_BINARY_DIR}/config.h ESCAPE_QUOTES)

add_executable(
  test src/test.cpp src/aseba_node.cpp src/aseba_default_description.c
       src/aseba_network.cpp src/aseba_script.cpp)
target_compile_definitions(test PUBLIC -DLOG_PRINT)
target_link_libraries(
  test
  ${LIBXML2_LIBRARIES}
  dashel
  asebacommon
  asebavmbuffer
  asebavm
  asebacompiler
  ${EXTRA_LIBS})

if(HAS_ZEROCONF_SUPPORT)
  add_executable(aseba_register src/register.cpp)
  target_compile_definitions(aseba_register PUBLIC -DLOG_PRINT)
  target_link_libraries(aseba_register asebacommon asebazeroconf)
  target_include_directories(
    aseba_register
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/aseba/aseba/common/zeroconf)
  install(TARGETS aseba_register DESTINATION ${COPPELIASIM_LIBRARIES_DIR})
  set_target_properties(aseba_register
                        PROPERTIES INSTALL_RPATH "${COPPELIASIM_LIBRARIES_DIR}")
  add_compile_definitions(
    REGISTER_CMD="${COPPELIASIM_LIBRARIES_DIR}/aseba_register")
endif()

coppeliasim_add_plugin(
  ${_PLUGIN_NAME}
  SOURCES
  src/plugin.cpp
  src/coppeliasim_robot.cpp
  src/coppeliasim_thymio2.cpp
  src/coppeliasim_epuck.cpp
  src/aseba_node.cpp
  src/aseba_thymio2_descriptions.c
  src/aseba_thymio2_natives.cpp
  src/aseba_thymio2.cpp
  src/aseba_default_description.c
  src/aseba_network.cpp
  src/aseba_script.cpp
  src/aseba_epuck_descriptions.c
  src/aseba_epuck_natives.cpp
  src/aseba_epuck.cpp)

# target_compile_definitions(${_PLUGIN_NAME} PUBLIC -DLOG_PRINT)

if(WIN32)
  set(EXPORT_ set)
else()
  set(EXPORT_ export)
endif()

find_package(Python3 REQUIRED COMPONENTS Interpreter)
add_custom_target(
  generate_thymio_lua ALL
  COMMAND
    ${EXPORT_}
    "PYTHONPATH=${LIBPLUGIN_DIR}/simStubsGen:${COPPELIASIM_INCLUDE_DIR}/simStubsGen"
    && ${Python3_EXECUTABLE}
    ${CMAKE_CURRENT_SOURCE_DIR}/helpers/generate_split_reference.py --directory
    ${CMAKE_CURRENT_BINARY_DIR}/generated --prefix ${PREFIX} &&
    ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/helpers/generate_lua.py
    Thymio _thymio2_ --directory ${CMAKE_CURRENT_BINARY_DIR}/generated &&
    ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/helpers/generate_lua.py
    EPuck _epuck_ --directory ${CMAKE_CURRENT_BINARY_DIR}/generated
  BYPRODUCTS ${CMAKE_CURRENT_BINARY_DIR}/generated/simThymio-typecheck.lua
             ${CMAKE_CURRENT_BINARY_DIR}/generated/simEPuck-typecheck.lua
  DEPENDS ${_PLUGIN_NAME}
  COMMENT "Generating Thymio and e-puck lua script")

add_custom_target(
  docs
  COMMAND
    ${EXPORT_}
    "PYTHONPATH=${LIBPLUGIN_DIR}/simStubsGen:${COPPELIASIM_INCLUDE_DIR}/simStubsGen"
    && ${Python3_EXECUTABLE}
    ${CMAKE_CURRENT_SOURCE_DIR}/helpers/generate_markdown_api.py --directory
    ${CMAKE_CURRENT_BINARY_DIR}/generated --file reference_Aseba.xml &&
    ${Python3_EXECUTABLE}
    ${CMAKE_CURRENT_SOURCE_DIR}/helpers/generate_markdown_api.py --directory
    ${CMAKE_CURRENT_BINARY_DIR}/generated --file reference_Thymio.xml &&
    ${Python3_EXECUTABLE}
    ${CMAKE_CURRENT_SOURCE_DIR}/helpers/generate_markdown_api.py --directory
    ${CMAKE_CURRENT_BINARY_DIR}/generated --file reference_EPuck.xml
  DEPENDS generate_thymio_lua
  COMMENT "Generating docs")

target_link_libraries(
  ${_PLUGIN_NAME}
  ${_KEYWORD}
  ${OpenCV_LIBS}
  ${LIBXML2_LIBRARIES}
  dashel
  asebacommon
  asebavmbuffer
  asebavm
  asebacompiler
  ${EXTRA_LIBS})

if(DEFINED MODEL_DIR)
  install(
    FILES ${CMAKE_CURRENT_SOURCE_DIR}/models/${MODEL_VERSION}/thymio.ttm
          ${CMAKE_CURRENT_SOURCE_DIR}/models/${MODEL_VERSION}/mighty_thymio.ttm
          ${CMAKE_CURRENT_SOURCE_DIR}/models/thymio-body-diffusionMap0.png
          ${CMAKE_CURRENT_SOURCE_DIR}/models/thymio-body-diffusionMap1.png
          ${CMAKE_CURRENT_SOURCE_DIR}/models/thymio-body-diffusionMap2.png
          ${CMAKE_CURRENT_SOURCE_DIR}/models/thymio-body-texture.png
          ${CMAKE_CURRENT_SOURCE_DIR}/models/${MODEL_VERSION}/e-puck-aseba.ttm
          ${CMAKE_CURRENT_SOURCE_DIR}/models/epuck.png
    DESTINATION ${MODEL_DIR})
endif()

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/generated/simThymio-typecheck.lua
              ${CMAKE_CURRENT_SOURCE_DIR}/lua/simThymio.lua
        DESTINATION ${COPPELIASIM_LUA_DIR})

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/generated/simEPuck-typecheck.lua
              ${CMAKE_CURRENT_SOURCE_DIR}/lua/simEPuck.lua
        DESTINATION ${COPPELIASIM_LUA_DIR})

install(TARGETS dashel asebacommon asebavmbuffer asebavm asebacompiler
                ${EXTRA_LIBS} DESTINATION ${COPPELIASIM_LIBRARIES_DIR})
