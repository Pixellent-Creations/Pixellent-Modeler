#include "PixellentModeler/Rendering/ShaderLibrary.hpp"
#include "PixellentModeler/Rendering/Shader.hpp"
#include <iostream>
#include <stdexcept>

namespace PixellentModeler {

void ShaderLibrary::load(const std::string& name, const std::string& vertexPath, const std::string& fragmentPath) {
    auto shader = std::make_shared<Shader>();
    if (shader->createFromFiles(vertexPath, fragmentPath)) {
        m_shaders[name] = std::move(shader);
    } else {
        std::cerr << "ShaderLibrary: failed to load shader '" << name << "'" << std::endl;
    }
}

void ShaderLibrary::add(const std::string& name, std::shared_ptr<Shader> shader) {
    if (m_shaders.count(name)) {
        std::cerr << "ShaderLibrary: shader '" << name << "' already exists, overwriting" << std::endl;
    }
    m_shaders[name] = std::move(shader);
}

std::shared_ptr<Shader> ShaderLibrary::get(const std::string& name) const {
    auto it = m_shaders.find(name);
    if (it == m_shaders.end()) {
        std::cerr << "ShaderLibrary: shader '" << name << "' not found" << std::endl;
        return nullptr;
    }
    return it->second;
}

bool ShaderLibrary::exists(const std::string& name) const {
    return m_shaders.find(name) != m_shaders.end();
}

} // namespace PixellentModeler
