#pragma once

#include <cstdint>
#include <string>
#include <unordered_map>
#include <glm/glm.hpp>

namespace PixellentModeler {

class Shader {
public:
    Shader() = default;
    ~Shader();

    Shader(const Shader&) = delete;
    Shader& operator=(const Shader&) = delete;
    Shader(Shader&& other) noexcept;
    Shader& operator=(Shader&& other) noexcept;

    bool create(const std::string& vertexSrc, const std::string& fragmentSrc);
    bool createFromFiles(const std::string& vertexPath, const std::string& fragmentPath);
    void bind() const;
    void unbind() const;

    void setInt(const std::string& name, int value);
    void setFloat(const std::string& name, float value);
    void setVec2(const std::string& name, const glm::vec2& value);
    void setVec3(const std::string& name, const glm::vec3& value);
    void setVec4(const std::string& name, const glm::vec4& value);
    void setMat3(const std::string& name, const glm::mat3& matrix);
    void setMat4(const std::string& name, const glm::mat4& matrix);

    uint32_t id() const { return m_program; }

private:
    uint32_t m_program = 0;
    mutable std::unordered_map<std::string, int> m_uniformCache;

    uint32_t compile(uint32_t type, const std::string& source);
    int getUniformLocation(const std::string& name) const;
};

} // namespace PixellentModeler
