cmake_minimum_required(VERSION 3.16)
project(BaseEngine LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(SFML 3 REQUIRED COMPONENTS System Window)
find_package(OpenGL REQUIRED)

# --- engine target ---
add_executable(engine
    src/main.cpp
    third_party/glad/src/gl.c
)

target_include_directories(engine PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/glad/include
)

target_link_libraries(engine PRIVATE
    SFML::System
    SFML::Window
    OpenGL::GL
)

# --- triangle demo target ---
add_executable(triangle
    src/demos/triangle.cpp
    third_party/glad/src/gl.c
)

target_include_directories(triangle PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/glad/include
)

target_link_libraries(triangle PRIVATE
    SFML::System
    SFML::Window
    OpenGL::GL
)
