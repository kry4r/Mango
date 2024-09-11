cmake_minimum_required(VERSION 3.10)
project(Mango)

set(CMAKE_CXX_STANDARD 23)

add_subdirectory(3rdparty/glfw)
add_subdirectory(3rdparty/glad)
add_subdirectory(3rdparty/imgui)
add_subdirectory(3rdparty/glm)

file(GLOB SOURCES
        src/*.cpp
        src/headers/*.hpp)
add_executable(Mango Mango.cpp ${SOURCES})
target_include_directories(Mango PRIVATE src/headers)
target_link_libraries(Mango glfw glad imgui glm)