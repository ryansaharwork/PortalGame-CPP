# Require CMake 3.31 or higher
cmake_minimum_required(VERSION 3.31)

# Define the project name (Lab12 here)
project(Lab12)

# Build for C++20
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_SCAN_FOR_MODULES OFF)

# Add dependencies (helper script)
include(../cmake/AddDependencies.cmake)

# This grabs all files in this directory that end with .cpp, .h, and .hpp
file(GLOB SOURCE_FILES CONFIGURE_DEPENDS "*.cpp" "*.h" "*.hpp")
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${SOURCE_FILES})

# Enable some specific compile flags on Windows/Mac
include(../cmake/SetCompileFlags.cmake)

# Build target and specify source files
add_executable(${CMAKE_PROJECT_NAME} ${SOURCE_FILES}
        VOTrigger.h
        HUD.cpp)

# Add additional include directories
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE ../External)

# Link against dependencies
target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE SDL3_mixer::SDL3_mixer SDL3::SDL3 SDL3_ttf::SDL3_ttf)

# Emscripten already includes GLEW support natively
if (NOT EMSCRIPTEN)
    target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE libglew_static)
endif()

# On Windows, we also need to copy the DLLs to the target directory
if(WIN32)
    add_custom_command(TARGET ${CMAKE_PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy -t $<TARGET_FILE_DIR:${CMAKE_PROJECT_NAME}> $<TARGET_RUNTIME_DLLS:${CMAKE_PROJECT_NAME}>
        COMMAND_EXPAND_LISTS)
endif()
