#pragma once

#include <string>
#include <glm/glm.hpp>

namespace PixellentModeler {

class Shader {
public:
    Shader() = default;
    ~Shader();

    // create shader from vertex/fragment source code
    bool create(const std::string& vertexSrc, const std::string& fragmentSrc);
    void bind() const;
    void unbind() const;

    // utility uniforms
    void setMat4(const std::string& name, const glm::mat4& matrix);

private:
    unsigned int m_program = 0;

    unsigned int compile(unsigned int type, const std::string& source);
    int getUniformLocation(const std::string& name) const;
};

} // namespace PixellentModeler
