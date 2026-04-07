#include "PixellentModeler/IO/ObjImporter.hpp"
#include "PixellentModeler/Mesh/HalfEdgeMesh.hpp"
#include "PixellentModeler/Scene/Scene.hpp"
#include "PixellentModeler/Scene/Entity.hpp"
#include "PixellentModeler/Scene/TransformComponent.hpp"
#include "PixellentModeler/Scene/MeshComponent.hpp"
#include "PixellentModeler/Scene/MaterialComponent.hpp"

#include <tiny_obj_loader.h>
#include <glm/glm.hpp>
#include <iostream>
#include <unordered_map>

namespace PixellentModeler {

ObjImporter::ImportResult ObjImporter::import(const std::string& path) {
    ImportResult result;

    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string warn, err;

    bool ok = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, path.c_str());

    if (!warn.empty()) {
        std::cerr << "[ObjImporter] Warning: " << warn << std::endl;
    }
    if (!err.empty()) {
        result.error = err;
        std::cerr << "[ObjImporter] Error: " << err << std::endl;
    }
    if (!ok) {
        result.success = false;
        if (result.error.empty()) {
            result.error = "Failed to load OBJ file: " + path;
        }
        return result;
    }

    // Process each shape into a separate HalfEdgeMesh
    for (const auto& shape : shapes) {
        auto mesh = std::make_shared<HalfEdgeMesh>();

        // We need to deduplicate vertices since OBJ uses separate indices
        // for positions, normals, and UVs
        struct VertexKey {
            int vi, ni, ti; // vertex, normal, texcoord indices
            bool operator==(const VertexKey& other) const {
                return vi == other.vi && ni == other.ni && ti == other.ti;
            }
        };
        struct VertexKeyHash {
            size_t operator()(const VertexKey& k) const {
                size_t h = std::hash<int>()(k.vi);
                h ^= std::hash<int>()(k.ni) + 0x9e3779b9 + (h << 6) + (h >> 2);
                h ^= std::hash<int>()(k.ti) + 0x9e3779b9 + (h << 6) + (h >> 2);
                return h;
            }
        };

        std::unordered_map<VertexKey, uint32_t, VertexKeyHash> vertexMap;
        std::vector<glm::vec3> vertices;
        std::vector<glm::vec3> normals;
        std::vector<glm::vec2> uvs;
        std::vector<std::vector<uint32_t>> faces;

        size_t indexOffset = 0;
        for (size_t f = 0; f < shape.mesh.num_face_vertices.size(); f++) {
            int fv = shape.mesh.num_face_vertices[f];
            std::vector<uint32_t> face;

            for (int v = 0; v < fv; v++) {
                tinyobj::index_t idx = shape.mesh.indices[indexOffset + v];

                VertexKey key;
                key.vi = idx.vertex_index;
                key.ni = idx.normal_index;
                key.ti = idx.texcoord_index;

                auto it = vertexMap.find(key);
                uint32_t vertexIndex;
                if (it != vertexMap.end()) {
                    vertexIndex = it->second;
                } else {
                    vertexIndex = static_cast<uint32_t>(vertices.size());
                    vertexMap[key] = vertexIndex;

                    // Position
                    glm::vec3 pos(0.0f);
                    if (idx.vertex_index >= 0) {
                        pos.x = attrib.vertices[3 * idx.vertex_index + 0];
                        pos.y = attrib.vertices[3 * idx.vertex_index + 1];
                        pos.z = attrib.vertices[3 * idx.vertex_index + 2];
                    }
                    vertices.push_back(pos);

                    // Normal
                    glm::vec3 normal(0.0f, 1.0f, 0.0f);
                    if (idx.normal_index >= 0 && !attrib.normals.empty()) {
                        normal.x = attrib.normals[3 * idx.normal_index + 0];
                        normal.y = attrib.normals[3 * idx.normal_index + 1];
                        normal.z = attrib.normals[3 * idx.normal_index + 2];
                    }
                    normals.push_back(normal);

                    // UV
                    glm::vec2 uv(0.0f);
                    if (idx.texcoord_index >= 0 && !attrib.texcoords.empty()) {
                        uv.x = attrib.texcoords[2 * idx.texcoord_index + 0];
                        uv.y = attrib.texcoords[2 * idx.texcoord_index + 1];
                    }
                    uvs.push_back(uv);
                }

                face.push_back(vertexIndex);
            }

            faces.push_back(face);
            indexOffset += fv;
        }

        // Build the HalfEdgeMesh from extracted polygon data
        mesh->buildFromPolygons(vertices, faces);
        mesh->setNormals(normals);
        mesh->setUVs(uvs);

        result.meshes.push_back(mesh);
        result.names.push_back(shape.name.empty() ? "Unnamed" : shape.name);
    }

    result.success = true;
    return result;
}

void ObjImporter::importToScene(const std::string& path, Scene& scene) {
    ImportResult result = import(path);
    if (!result.success) {
        std::cerr << "[ObjImporter] Failed to import to scene: " << result.error << std::endl;
        return;
    }

    for (size_t i = 0; i < result.meshes.size(); i++) {
        const std::string& name = result.names[i];
        Entity& entity = scene.createEntity(name);

        // Add transform (default identity)
        auto& transform = entity.addComponent<TransformComponent>();
        transform.position = glm::vec3(0.0f);
        transform.rotation = glm::vec3(0.0f);
        transform.scale = glm::vec3(1.0f);

        // Add mesh
        auto& meshComp = entity.addComponent<MeshComponent>();
        meshComp.mesh = result.meshes[i];

        // Add default material
        auto& material = entity.addComponent<MaterialComponent>();
        material.albedo = glm::vec3(0.8f, 0.8f, 0.8f);
        material.metallic = 0.0f;
        material.roughness = 0.5f;
    }
}

} // namespace PixellentModeler
