#include "PixellentModeler/Rendering/Shader.hpp"
#include <glad/glad.h>
#include <glm/gtc/type_ptr.hpp>
#include <fstream>
#include <sstream>
#include <iostream>
#include <utility>

namespace PixellentModeler {

Shader::~Shader() {
    if (m_program) {
        glDeleteProgram(m_program);
    }
}

Shader::Shader(Shader&& other) noexcept
    : m_program(other.m_program), m_uniformCache(std::move(other.m_uniformCache)) {
    other.m_program = 0;
}

Shader& Shader::operator=(Shader&& other) noexcept {
    if (this != &other) {
        if (m_program) {
            glDeleteProgram(m_program);
        }
        m_program = other.m_program;
        m_uniformCache = std::move(other.m_uniformCache);
        other.m_program = 0;
    }
    return *this;
}

uint32_t Shader::compile(uint32_t type, const std::string& source) {
    uint32_t id = glCreateShader(type);
    const char* src = source.c_str();
    glShaderSource(id, 1, &src, nullptr);
    glCompileShader(id);

    int result;
    glGetShaderiv(id, GL_COMPILE_STATUS, &result);
    if (result == GL_FALSE) {
        int length;
        glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length);
        std::string message(static_cast<size_t>(length), ' ');
        glGetShaderInfoLog(id, length, &length, &message[0]);
        std::cerr << "Failed to compile "
                  << (type == GL_VERTEX_SHADER ? "vertex" : "fragment")
                  << " shader: " << message << std::endl;
        glDeleteShader(id);
        return 0;
    }

    return id;
}

bool Shader::create(const std::string& vertexSrc, const std::string& fragmentSrc) {
    uint32_t program = glCreateProgram();
    uint32_t vs = compile(GL_VERTEX_SHADER, vertexSrc);
    uint32_t fs = compile(GL_FRAGMENT_SHADER, fragmentSrc);

    if (!vs || !fs) {
        if (vs) glDeleteShader(vs);
        if (fs) glDeleteShader(fs);
        glDeleteProgram(program);
        return false;
    }

    glAttachShader(program, vs);
    glAttachShader(program, fs);
    glLinkProgram(program);

    int linkStatus;
    glGetProgramiv(program, GL_LINK_STATUS, &linkStatus);
    if (linkStatus == GL_FALSE) {
        int length;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &length);
        std::string message(static_cast<size_t>(length), ' ');
        glGetProgramInfoLog(program, length, &length, &message[0]);
        std::cerr << "Failed to link shader program: " << message << std::endl;
        glDeleteShader(vs);
        glDeleteShader(fs);
        glDeleteProgram(program);
        return false;
    }

    glValidateProgram(program);
    glDeleteShader(vs);
    glDeleteShader(fs);

    // Delete old program if exists
    if (m_program) {
        glDeleteProgram(m_program);
    }
    m_program = program;
    m_uniformCache.clear();
    return true;
}

bool Shader::createFromFiles(const std::string& vertexPath, const std::string& fragmentPath) {
    std::ifstream vertFile(vertexPath);
    if (!vertFile.is_open()) {
        std::cerr << "Failed to open vertex shader file: " << vertexPath << std::endl;
        return false;
    }

    std::ifstream fragFile(fragmentPath);
    if (!fragFile.is_open()) {
        std::cerr << "Failed to open fragment shader file: " << fragmentPath << std::endl;
        return false;
    }

    std::stringstream vertStream, fragStream;
    vertStream << vertFile.rdbuf();
    fragStream << fragFile.rdbuf();

    return create(vertStream.str(), fragStream.str());
}

void Shader::bind() const {
    glUseProgram(m_program);
}

void Shader::unbind() const {
    glUseProgram(0);
}

int Shader::getUniformLocation(const std::string& name) const {
    auto it = m_uniformCache.find(name);
    if (it != m_uniformCache.end()) {
        return it->second;
    }
    int location = glGetUniformLocation(m_program, name.c_str());
    m_uniformCache[name] = location;
    return location;
}

void Shader::setInt(const std::string& name, int value) {
    int loc = getUniformLocation(name);
    if (loc != -1) {
        glUniform1i(loc, value);
    }
}

void Shader::setFloat(const std::string& name, float value) {
    int loc = getUniformLocation(name);
    if (loc != -1) {
        glUniform1f(loc, value);
    }
}

void Shader::setVec2(const std::string& name, const glm::vec2& value) {
    int loc = getUniformLocation(name);
    if (loc != -1) {
        glUniform2fv(loc, 1, glm::value_ptr(value));
    }
}

void Shader::setVec3(const std::string& name, const glm::vec3& value) {
    int loc = getUniformLocation(name);
    if (loc != -1) {
        glUniform3fv(loc, 1, glm::value_ptr(value));
    }
}

void Shader::setVec4(const std::string& name, const glm::vec4& value) {
    int loc = getUniformLocation(name);
    if (loc != -1) {
        glUniform4fv(loc, 1, glm::value_ptr(value));
    }
}

void Shader::setMat3(const std::string& name, const glm::mat3& matrix) {
    int loc = getUniformLocation(name);
    if (loc != -1) {
        glUniformMatrix3fv(loc, 1, GL_FALSE, glm::value_ptr(matrix));
    }
}

void Shader::setMat4(const std::string& name, const glm::mat4& matrix) {
    int loc = getUniformLocation(name);
    if (loc != -1) {
        glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(matrix));
    }
}

} // namespace PixellentModeler
