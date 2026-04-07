#include "PixellentModeler/IO/ProjectSerializer.hpp"
#include "PixellentModeler/Mesh/HalfEdgeMesh.hpp"
#include "PixellentModeler/Scene/Scene.hpp"
#include "PixellentModeler/Scene/Entity.hpp"
#include "PixellentModeler/Scene/TransformComponent.hpp"
#include "PixellentModeler/Scene/MeshComponent.hpp"
#include "PixellentModeler/Scene/MaterialComponent.hpp"
#include "PixellentModeler/Scene/LightComponent.hpp"

#include <nlohmann/json.hpp>
#include <glm/glm.hpp>
#include <fstream>
#include <iostream>

using json = nlohmann::json;

namespace PixellentModeler {

// ---------------------------------------------------------------------------
// Helpers for glm <-> JSON conversion
// ---------------------------------------------------------------------------

static json vec3ToJson(const glm::vec3& v) {
    return json::array({v.x, v.y, v.z});
}

static glm::vec3 jsonToVec3(const json& j) {
    return glm::vec3(j[0].get<float>(), j[1].get<float>(), j[2].get<float>());
}

static json vec2ToJson(const glm::vec2& v) {
    return json::array({v.x, v.y});
}

static glm::vec2 jsonToVec2(const json& j) {
    return glm::vec2(j[0].get<float>(), j[1].get<float>());
}

// ---------------------------------------------------------------------------
// Save
// ---------------------------------------------------------------------------

bool ProjectSerializer::save(const Scene& scene, const std::string& path) {
    json root;
    root["version"] = "1.0";

    json sceneJson;

    // Serialize camera
    const auto& cam = scene.camera();
    json cameraJson;
    cameraJson["position"] = vec3ToJson(cam.getPosition());
    cameraJson["target"] = vec3ToJson(cam.getTarget());
    cameraJson["fov"] = cam.getFov();
    cameraJson["distance"] = cam.getDistance();
    sceneJson["camera"] = cameraJson;

    // Serialize entities
    json entitiesJson = json::array();
    const auto& entities = scene.getEntities();

    for (const auto& entity : entities) {
        json entityJson;
        entityJson["id"] = entity->id();
        entityJson["name"] = entity->name();

        // Transform
        if (entity->hasComponent<TransformComponent>()) {
            const auto* transform = entity->getComponent<TransformComponent>();
            json transformJson;
            transformJson["position"] = vec3ToJson(transform->position);
            transformJson["rotation"] = vec3ToJson(transform->rotation);
            transformJson["scale"] = vec3ToJson(transform->scale);
            entityJson["transform"] = transformJson;
        } else {
            entityJson["transform"] = nullptr;
        }

        // Mesh
        if (entity->hasComponent<MeshComponent>()) {
            const auto* meshComp = entity->getComponent<MeshComponent>();
            if (meshComp && meshComp->mesh) {
                json meshJson;

                // Vertices
                auto verts = meshComp->mesh->getVertices();
                json vertsJson = json::array();
                for (const auto& v : verts) {
                    vertsJson.push_back(vec3ToJson(v));
                }
                meshJson["vertices"] = vertsJson;

                // Faces
                auto faces = meshComp->mesh->getFaceVertexIndices();
                json facesJson = json::array();
                for (const auto& face : faces) {
                    json faceJson = json::array();
                    for (uint32_t idx : face) {
                        faceJson.push_back(idx);
                    }
                    facesJson.push_back(faceJson);
                }
                meshJson["faces"] = facesJson;

                // UVs
                auto uvs = meshComp->mesh->getUVs();
                json uvsJson = json::array();
                for (const auto& uv : uvs) {
                    uvsJson.push_back(vec2ToJson(uv));
                }
                meshJson["uvs"] = uvsJson;

                entityJson["mesh"] = meshJson;
            } else {
                entityJson["mesh"] = nullptr;
            }
        } else {
            entityJson["mesh"] = nullptr;
        }

        // Material
        if (entity->hasComponent<MaterialComponent>()) {
            const auto* material = entity->getComponent<MaterialComponent>();
            json matJson;
            matJson["albedo"] = vec3ToJson(material->albedo);
            matJson["metallic"] = material->metallic;
            matJson["roughness"] = material->roughness;
            entityJson["material"] = matJson;
        } else {
            entityJson["material"] = nullptr;
        }

        // Light
        if (entity->hasComponent<LightComponent>()) {
            const auto* light = entity->getComponent<LightComponent>();
            json lightJson;
            lightJson["type"] = static_cast<int>(light->type);
            lightJson["color"] = vec3ToJson(light->color);
            lightJson["intensity"] = light->intensity;
            entityJson["light"] = lightJson;
        } else {
            entityJson["light"] = nullptr;
        }

        entitiesJson.push_back(entityJson);
    }

    sceneJson["entities"] = entitiesJson;
    root["scene"] = sceneJson;

    // Write to file
    std::ofstream out(path);
    if (!out.is_open()) {
        std::cerr << "[ProjectSerializer] Failed to open file for writing: " << path << std::endl;
        return false;
    }

    out << root.dump(4);
    out.close();
    return true;
}

// ---------------------------------------------------------------------------
// Load
// ---------------------------------------------------------------------------

bool ProjectSerializer::load(Scene& scene, const std::string& path) {
    std::ifstream in(path);
    if (!in.is_open()) {
        std::cerr << "[ProjectSerializer] Failed to open file for reading: " << path << std::endl;
        return false;
    }

    json root;
    try {
        in >> root;
    } catch (const json::parse_error& e) {
        std::cerr << "[ProjectSerializer] JSON parse error: " << e.what() << std::endl;
        return false;
    }
    in.close();

    // Version check
    std::string version = root.value("version", "1.0");
    if (version != "1.0") {
        std::cerr << "[ProjectSerializer] Warning: unknown version '" << version
                  << "', attempting to load anyway." << std::endl;
    }

    if (!root.contains("scene")) {
        std::cerr << "[ProjectSerializer] Missing 'scene' key in project file." << std::endl;
        return false;
    }

    const auto& sceneJson = root["scene"];

    // Clear existing scene
    scene.clear();

    // Load camera
    if (sceneJson.contains("camera")) {
        const auto& camJson = sceneJson["camera"];
        auto& cam = scene.camera();

        if (camJson.contains("position")) {
            cam.setPosition(jsonToVec3(camJson["position"]));
        }
        if (camJson.contains("target")) {
            glm::vec3 target = jsonToVec3(camJson["target"]);
            cam.lookAt(target);
        }
        if (camJson.contains("fov")) {
            cam.setFov(camJson["fov"].get<float>());
        }
        if (camJson.contains("distance")) {
            cam.setDistance(camJson["distance"].get<float>());
        }
    }

    // Load entities
    if (sceneJson.contains("entities")) {
        for (const auto& entityJson : sceneJson["entities"]) {
            std::string name = entityJson.value("name", "Unnamed");
            Entity& entity = scene.createEntity(name);

            // Transform
            if (entityJson.contains("transform") && !entityJson["transform"].is_null()) {
                const auto& transformJson = entityJson["transform"];
                auto& transform = entity.addComponent<TransformComponent>();
                transform.position = jsonToVec3(transformJson["position"]);
                transform.rotation = jsonToVec3(transformJson["rotation"]);
                transform.scale = jsonToVec3(transformJson["scale"]);
            }

            // Mesh
            if (entityJson.contains("mesh") && !entityJson["mesh"].is_null()) {
                const auto& meshJson = entityJson["mesh"];
                auto& meshComp = entity.addComponent<MeshComponent>();
                meshComp.mesh = std::make_shared<HalfEdgeMesh>();

                // Read vertices
                std::vector<glm::vec3> vertices;
                if (meshJson.contains("vertices")) {
                    for (const auto& vJson : meshJson["vertices"]) {
                        vertices.push_back(jsonToVec3(vJson));
                    }
                }

                // Read faces
                std::vector<std::vector<uint32_t>> faces;
                if (meshJson.contains("faces")) {
                    for (const auto& fJson : meshJson["faces"]) {
                        std::vector<uint32_t> face;
                        for (const auto& idx : fJson) {
                            face.push_back(idx.get<uint32_t>());
                        }
                        faces.push_back(face);
                    }
                }

                // Build mesh topology
                meshComp.mesh->buildFromPolygons(vertices, faces);

                // Read UVs
                if (meshJson.contains("uvs")) {
                    std::vector<glm::vec2> uvs;
                    for (const auto& uvJson : meshJson["uvs"]) {
                        uvs.push_back(jsonToVec2(uvJson));
                    }
                    meshComp.mesh->setUVs(uvs);
                }
            }

            // Material
            if (entityJson.contains("material") && !entityJson["material"].is_null()) {
                const auto& matJson = entityJson["material"];
                auto& material = entity.addComponent<MaterialComponent>();
                material.albedo = jsonToVec3(matJson["albedo"]);
                material.metallic = matJson.value("metallic", 0.0f);
                material.roughness = matJson.value("roughness", 0.5f);
            }

            // Light
            if (entityJson.contains("light") && !entityJson["light"].is_null()) {
                const auto& lightJson = entityJson["light"];
                auto& light = entity.addComponent<LightComponent>();
                light.type = static_cast<LightType>(lightJson.value("type", 0));
                light.color = jsonToVec3(lightJson["color"]);
                light.intensity = lightJson.value("intensity", 1.0f);
            }
        }
    }

    return true;
}

} // namespace PixellentModeler
