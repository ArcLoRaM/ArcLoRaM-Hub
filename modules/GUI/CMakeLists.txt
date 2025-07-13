
cmake_minimum_required(VERSION 3.16)
project(ARCLORAMGUI)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/build/exec)

# === Default to Debug if not set ===
# if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_BUILD_TYPE Debug)
# endif()


# Output directory
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/build/exec)


include(FetchContent)


# Set SFML and TGUI paths
set(SFML_ROOT "C:/Users/srcla/Desktop/ArcLoRaM/ArcLoRaM-Hub/modules/libraries/SFML-3.0.0")
set(SFML_LIB_DIR "${SFML_ROOT}/lib")
include_directories("${SFML_ROOT}/include")

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(SFML_LIBS
        sfml-graphics-d
        sfml-window-d
        sfml-system-d
        sfml-network-d
    )
else()
    set(SFML_LIBS
        sfml-graphics
        sfml-window
        sfml-system
        sfml-network
    )
endif()


# === TGUI CONFIGURATION ===
set(BUILD_SHARED_LIBS ON)
set(TGUI_BACKEND SFML_GRAPHICS)
set(TGUI_SKIP_SFML_FIND_PACKAGE TRUE) # Don't let TGUI use FetchContent to build its own SFML


FetchContent_Declare(
  TGUI
  GIT_REPOSITORY https://github.com/texus/TGUI
  GIT_TAG v1.10.0
)
FetchContent_MakeAvailable(TGUI)


# === Include Paths ===
include_directories(${PROJECT_SOURCE_DIR}/src/Shared/Libraries)

# === SOURCES ===
file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS
    ${PROJECT_SOURCE_DIR}/src/*.cpp
)

add_executable(ARCLORAMGUI ${SOURCES})

# === LINKING ===
target_link_libraries(ARCLORAMGUI
  PRIVATE
  TGUI::TGUI
  "${SFML_ROOT}/lib/libsfml-graphics-d.a"
    "${SFML_ROOT}/lib/libsfml-window-d.a"
    "${SFML_ROOT}/lib/libsfml-system-d.a"
    "${SFML_ROOT}/lib/libsfml-network-d.a"
)
