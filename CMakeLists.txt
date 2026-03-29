cmake_minimum_required(VERSION 3.15)
project(BurgerDoom)

# Where CMake will search for .cmake module files
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

# Causes CMake projects to go into their own folders
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# Include common build scripts and do basic build setup
include(HelperFunctions)
include(BuildSetup)

build_setup()

# Global identifiers for each project/target
set(GAME_NAME BurgerDoom)

# Adding individual projects and libraries
set(GAME_SRC_DIR "${PROJECT_SOURCE_DIR}/source")

if (PLATFORM_UWP)
    set(SDL2_DIR "${PROJECT_SOURCE_DIR}/3rdparty/SDL2-UWP/share/sdl2")
endif()

add_subdirectory("${GAME_SRC_DIR}")

if (NOT PLATFORM_UWP)
    # Include the FindSDL2 module from the cmake directory
    list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
    find_package(SDL2 REQUIRED)

    if (SDL2_FOUND)
        include_directories(${SDL2_INCLUDE_DIRS})
        target_link_libraries(${GAME_NAME} ${SDL2_LIBRARIES})
    else()
        message(FATAL_ERROR "SDL2 not found!")
    endif()
endif()