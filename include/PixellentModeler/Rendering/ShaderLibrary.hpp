#pragma once

#include <memory>
#include <string>
#include <unordered_map>

namespace PixellentModeler {

class Shader;

class ShaderLibrary {
public:
    void load(const std::string& name, const std::string& vertexPath, const std::string& fragmentPath);
    void add(const std::string& name, std::shared_ptr<Shader> shader);
    std::shared_ptr<Shader> get(const std::string& name) const;
    bool exists(const std::string& name) const;

private:
    std::unordered_map<std::string, std::shared_ptr<Shader>> m_shaders;
};

} // namespace PixellentModeler
