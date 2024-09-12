#include "shader.hpp"

namespace mango::shader
{
    void Shader_GL::set_shader(const GLchar* vertexPath, const GLchar* fragmentPath)
    {
        std::string vertex_code;
        std::string fragment_code;
        std::ifstream vert_shader_file;
        std::ifstream frag_shader_file;

        vert_shader_file.exceptions(std::ifstream::badbit);
        frag_shader_file.exceptions(std::ifstream::badbit);

        try
        {
            vert_shader_file.open(vertexPath);
            frag_shader_file.open(fragmentPath);
            std::stringstream vShaderStream, fShaderStream;

            vShaderStream << vert_shader_file.rdbuf();
            fShaderStream << frag_shader_file.rdbuf();
            vert_shader_file.close();
            frag_shader_file.close();

            vertex_code = vShaderStream.str();
            fragment_code = fShaderStream.str();
        }

        catch (std::ifstream::failure e)
        {
            std::cout << "ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ" << std::endl;
        }

        const GLchar* vShaderCode = vertex_code.c_str();
        const GLchar* fShaderCode = fragment_code.c_str();

        // Shaders compilation
        GLuint vertex, fragment;
        GLint success;
        GLchar infoLog[512];

        // Vertex Shader
        vertex = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertex, 1, &vShaderCode, NULL);
        glCompileShader(vertex);
        glGetShaderiv(vertex, GL_COMPILE_STATUS, &success);

        if (!success)
        {
            glGetShaderInfoLog(vertex, 512, NULL, infoLog);
            std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
        }

        // Fragment Shader
        fragment = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragment, 1, &fShaderCode, NULL);
        glCompileShader(fragment);
        glGetShaderiv(fragment, GL_COMPILE_STATUS, &success);

        if (!success)
        {
            glGetShaderInfoLog(fragment, 512, NULL, infoLog);
            std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
        }

        // Shader Program
        this->Program = glCreateProgram();
        glAttachShader(this->Program, vertex);
        glAttachShader(this->Program, fragment);
        glLinkProgram(this->Program);
        glGetProgramiv(this->Program, GL_LINK_STATUS, &success);

        if (!success)
        {
            glGetProgramInfoLog(this->Program, 512, NULL, infoLog);
            std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
        }

        glDeleteShader(vertex);
        glDeleteShader(fragment);
    }

    void Shader_GL::use_shader()
    {
        glUseProgram(this->Program);
    }
}