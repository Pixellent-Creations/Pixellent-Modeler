#pragma once

#include "PixellentModeler/Scene/Component.hpp"
#include "PixellentModeler/Mesh/HalfEdgeMesh.hpp"
#include <vector>
#include <memory>
#include <glm/glm.hpp>

namespace PixellentModeler {

class Modifier {
public:
    virtual ~Modifier() = default;
    virtual std::string name() const = 0;
    virtual HalfEdgeMesh apply(const HalfEdgeMesh& baseMesh) const = 0;
    virtual void setParameter(const std::string& key, float value) {}
};

class ArrayModifier : public Modifier {
public:
    std::string name() const override { return "Array"; }
    HalfEdgeMesh apply(const HalfEdgeMesh& baseMesh) const override;

    int countX = 2, countY = 1, countZ = 1;
    float offsetX = 1.0f, offsetY = 1.0f, offsetZ = 1.0f;
    bool merge = true;
};

class MirrorModifier : public Modifier {
public:
    std::string name() const override { return "Mirror"; }
    HalfEdgeMesh apply(const HalfEdgeMesh& baseMesh) const override;

    // 0 = X, 1 = Y, 2 = Z
    int axis = 0;
    float offset = 0.0f;
    float threshold = 0.001f;
};

class BevelModifier : public Modifier {
public:
    std::string name() const override { return "Bevel"; }
    HalfEdgeMesh apply(const HalfEdgeMesh& baseMesh) const override;

    float amount = 0.1f;
    bool vertexBevel = false;
    bool edgeBevel = true;
};

class ModifierComponent : public Component {
public:
    HalfEdgeMesh baseMesh;  // Original mesh (preserved)
    std::vector<std::shared_ptr<Modifier>> modifiers;

    HalfEdgeMesh computeModifiedMesh() const {
        HalfEdgeMesh result = baseMesh;
        for (const auto& mod : modifiers) {
            result = mod->apply(result);
        }
        return result;
    }

    void addModifier(std::shared_ptr<Modifier> mod) {
        if (mod) modifiers.push_back(mod);
    }

    void removeModifier(size_t index) {
        if (index < modifiers.size()) {
            modifiers.erase(modifiers.begin() + index);
        }
    }

    void moveModifier(size_t from, size_t to) {
        if (from < modifiers.size() && to < modifiers.size()) {
            auto mod = modifiers[from];
            modifiers.erase(modifiers.begin() + from);
            modifiers.insert(modifiers.begin() + to, mod);
        }
    }

    void applyModifiers() {
        // Bake modifiers into base mesh
        baseMesh = computeModifiedMesh();
        modifiers.clear();
    }
};

} // namespace PixellentModeler
