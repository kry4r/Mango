cmake_minimum_required(VERSION 3.10)
project(Mango)

set(CMAKE_CXX_STANDARD 23)
find_package(glad CONFIG REQUIRED)
find_package(Stb REQUIRED)
add_subdirectory(3rdparty/glfw)
add_subdirectory(3rdparty/imgui)
add_subdirectory(3rdparty/glm)


file(GLOB SOURCES
        src/*.cpp
        src/headers/*.hpp
        src/*.hpp)
add_executable(Mango Mango.cpp ${SOURCES})
target_include_directories(Mango PRIVATE src/headers ${Stb_INCLUDE_DIR})
target_link_libraries(Mango glfw glad::glad imgui glm)