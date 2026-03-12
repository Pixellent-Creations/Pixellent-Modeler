#include "PixellentModeler/Shader.hpp"
#include <glad/glad.h>
#include <iostream>

namespace PixellentModeler {

unsigned int Shader::compile(unsigned int type, const std::string& source) {
    unsigned int id = glCreateShader(type);
    const char* src = source.c_str();
    glShaderSource(id, 1, &src, nullptr);
    glCompileShader(id);

    int result;
    glGetShaderiv(id, GL_COMPILE_STATUS, &result);
    if (result == GL_FALSE) {
        int length;
        glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length);
        std::string message(length, ' ');
        glGetShaderInfoLog(id, length, &length, &message[0]);
        std::cerr << "Failed to compile shader: " << message << std::endl;
        glDeleteShader(id);
        return 0;
    }

    return id;
}

Shader::~Shader() {
    if (m_program)
        glDeleteProgram(m_program);
}

bool Shader::create(const std::string& vertexSrc, const std::string& fragmentSrc) {
    unsigned int program = glCreateProgram();
    unsigned int vs = compile(GL_VERTEX_SHADER, vertexSrc);
    unsigned int fs = compile(GL_FRAGMENT_SHADER, fragmentSrc);

    if (!vs || !fs) {
        glDeleteProgram(program);
        return false;
    }

    glAttachShader(program, vs);
    glAttachShader(program, fs);
    glLinkProgram(program);
    glValidateProgram(program);

    glDeleteShader(vs);
    glDeleteShader(fs);

    m_program = program;
    return true;
}

void Shader::bind() const {
    glUseProgram(m_program);
}

void Shader::unbind() const {
    glUseProgram(0);
}

int Shader::getUniformLocation(const std::string& name) const {
    return glGetUniformLocation(m_program, name.c_str());
}

void Shader::setMat4(const std::string& name, const glm::mat4& matrix) {
    int loc = getUniformLocation(name);
    if (loc != -1) {
        glUniformMatrix4fv(loc, 1, GL_FALSE, &matrix[0][0]);
    }
}

} // namespace PixellentModeler
