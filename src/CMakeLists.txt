set(LIB_NAME src)
file(GLOB SOURCES
        *.hpp
        *.cpp
)
add_library(${LIB_NAME} STATIC ${SOURCES})
target_include_directories(${LIB_NAME} PRIVATE headers ${Stb_INCLUDE_DIR} 3rdparty/glm/glm 3rdparty/imgui 3rdparty/glfw/include)
target_include_directories(${LIB_NAME} PRIVATE ${TINYGLTF_INCLUDE_DIRS})
target_link_libraries(${LIB_NAME} glfw glad::glad imgui glm assimp::assimp tinyobjloader::tinyobjloader)