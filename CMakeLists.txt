cmake_minimum_required(VERSION 3.16)
project(evil_nerds VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ---------------------------------------------------------------------------
# Исходники
# ---------------------------------------------------------------------------

set(SOURCES
    src/main.cpp
    src/loader.cpp
    src/world.cpp
    src/player.cpp
    src/combat.cpp
    src/input.cpp
    src/renderer.cpp
)

add_executable(evil_nerds ${SOURCES})

# ---------------------------------------------------------------------------
# Подключение nlohmann/json (header-only)
# ---------------------------------------------------------------------------

target_include_directories(evil_nerds PRIVATE include)

# ---------------------------------------------------------------------------
# Флаги компилятора
# ---------------------------------------------------------------------------

if (MSVC)
    # Windows (Visual Studio)
    target_compile_options(evil_nerds PRIVATE /W4 /utf-8)
else()
    # macOS / Linux (clang / gcc)
    target_compile_options(evil_nerds PRIVATE -Wall -Wextra -Wpedantic)
endif()

# ---------------------------------------------------------------------------
# Копирование папки assets рядом с исполняемым файлом после сборки
# ---------------------------------------------------------------------------

add_custom_command(TARGET evil_nerds POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/assets
        $<TARGET_FILE_DIR:evil_nerds>/assets
    COMMENT "Copying assets to build directory"
)
