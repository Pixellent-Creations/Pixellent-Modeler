#include "PixellentModeler/IO/ObjExporter.hpp"
#include "PixellentModeler/Mesh/HalfEdgeMesh.hpp"
#include "PixellentModeler/Scene/Scene.hpp"
#include "PixellentModeler/Scene/Entity.hpp"
#include "PixellentModeler/Scene/MeshComponent.hpp"

#include <glm/glm.hpp>
#include <fstream>
#include <iostream>
#include <iomanip>

namespace PixellentModeler {

static bool writeMeshToStream(std::ofstream& out, const HalfEdgeMesh& mesh,
                               const std::string& objectName,
                               uint32_t vertexOffset, uint32_t normalOffset,
                               uint32_t uvOffset) {
    const auto& vertices = mesh.getVertices();
    const auto& normals = mesh.getNormals();
    const auto& uvs = mesh.getUVs();
    auto faces = mesh.getFaceVertexIndices();

    bool hasNormals = !normals.empty();
    bool hasUVs = !uvs.empty();

    // Object name
    out << "o " << objectName << "\n";

    // Vertices
    for (const auto& v : vertices) {
        out << "v " << std::fixed << std::setprecision(6)
            << v.x << " " << v.y << " " << v.z << "\n";
    }

    // Normals
    if (hasNormals) {
        for (const auto& n : normals) {
            out << "vn " << std::fixed << std::setprecision(6)
                << n.x << " " << n.y << " " << n.z << "\n";
        }
    }

    // UVs
    if (hasUVs) {
        for (const auto& uv : uvs) {
            out << "vt " << std::fixed << std::setprecision(6)
                << uv.x << " " << uv.y << "\n";
        }
    }

    // Faces
    for (const auto& face : faces) {
        out << "f";
        for (uint32_t idx : face) {
            // OBJ indices are 1-based
            uint32_t vi = idx + vertexOffset + 1;

            if (hasUVs && hasNormals) {
                uint32_t ti = idx + uvOffset + 1;
                uint32_t ni = idx + normalOffset + 1;
                out << " " << vi << "/" << ti << "/" << ni;
            } else if (hasUVs) {
                uint32_t ti = idx + uvOffset + 1;
                out << " " << vi << "/" << ti;
            } else if (hasNormals) {
                uint32_t ni = idx + normalOffset + 1;
                out << " " << vi << "//" << ni;
            } else {
                out << " " << vi;
            }
        }
        out << "\n";
    }

    return true;
}

bool ObjExporter::exportMesh(const HalfEdgeMesh& mesh, const std::string& path) {
    std::ofstream out(path);
    if (!out.is_open()) {
        std::cerr << "[ObjExporter] Failed to open file for writing: " << path << std::endl;
        return false;
    }

    out << "# Exported by Pixellent Modeler\n";
    out << "# https://github.com/Pixellent-Creations/Pixellent-Modeler\n\n";

    if (!writeMeshToStream(out, mesh, "Object", 0, 0, 0)) {
        return false;
    }

    out.close();
    return true;
}

bool ObjExporter::exportScene(const Scene& scene, const std::string& path) {
    std::ofstream out(path);
    if (!out.is_open()) {
        std::cerr << "[ObjExporter] Failed to open file for writing: " << path << std::endl;
        return false;
    }

    out << "# Exported by Pixellent Modeler\n";
    out << "# https://github.com/Pixellent-Creations/Pixellent-Modeler\n\n";

    uint32_t vertexOffset = 0;
    uint32_t normalOffset = 0;
    uint32_t uvOffset = 0;

    const auto& entities = scene.getEntities();
    for (const auto& entity : entities) {
        if (!entity->hasComponent<MeshComponent>()) {
            continue;
        }

        const auto* meshComp = entity->getComponent<MeshComponent>();
        if (!meshComp || !meshComp->mesh) {
            continue;
        }

        const auto& mesh = *meshComp->mesh;
        std::string name = entity->name().empty() ? "Unnamed" : entity->name();

        if (!writeMeshToStream(out, mesh, name, vertexOffset, normalOffset, uvOffset)) {
            return false;
        }

        vertexOffset += static_cast<uint32_t>(mesh.getVertices().size());
        normalOffset += static_cast<uint32_t>(mesh.getNormals().size());
        uvOffset += static_cast<uint32_t>(mesh.getUVs().size());
    }

    out.close();
    return true;
}

} // namespace PixellentModeler
