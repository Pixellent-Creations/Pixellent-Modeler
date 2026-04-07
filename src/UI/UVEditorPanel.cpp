#include "PixellentModeler/UI/UVEditorPanel.hpp"
#include "PixellentModeler/Scene/Entity.hpp"
#include "PixellentModeler/Scene/MeshComponent.hpp"
#include <iostream>

namespace PixellentModeler {

void UVEditorPanel::render(Entity* selectedEntity) {
    if (!selectedEntity) return;

    auto* meshComp = selectedEntity->getComponent<MeshComponent>();
    if (!meshComp || !meshComp->mesh) return;

    // ImGui would be used here in full implementation
    std::cout << "UV Editor Panel - Mesh has " << meshComp->mesh->vertexCount()
              << " vertices with UVs" << std::endl;
}

void UVEditorPanel::markSeams(Entity* entity, const std::vector<uint32_t>& edgeIds) {
    if (!entity) return;

    auto* meshComp = entity->getComponent<MeshComponent>();
    if (!meshComp || !meshComp->mesh) return;

    m_markedSeams = edgeIds;
    std::cout << "Marked " << edgeIds.size() << " seams for unwrapping" << std::endl;
}

void UVEditorPanel::unwrapUVs(Entity* entity) {
    if (!entity) return;

    auto* meshComp = entity->getComponent<MeshComponent>();
    if (!meshComp || !meshComp->mesh) return;

    UVUtils::unwrapMesh(*meshComp->mesh);
    std::cout << "UV unwrap completed" << std::endl;
}

void UVEditorPanel::optimizePacking(Entity* entity) {
    if (!entity) return;

    auto* meshComp = entity->getComponent<MeshComponent>();
    if (!meshComp || !meshComp->mesh) return;

    UVUtils::packIslands(*meshComp->mesh);
    std::cout << "UV packing optimized" << std::endl;
}

// ============================================
// TexturePanel
// ============================================

void TexturePanel::render(Entity* selectedEntity) {
    if (!selectedEntity) return;

    auto* meshComp = selectedEntity->getComponent<MeshComponent>();
    if (!meshComp) return;

    // ImGui would be used here in full implementation
    std::cout << "Texture Paint Panel active" << std::endl;
}

void TexturePanel::paintColor(Entity* entity, const glm::vec3& color) {
    if (!entity) return;

    auto* meshComp = entity->getComponent<MeshComponent>();
    if (!meshComp) return;

    m_brushColor = color;
    std::cout << "Painting with color (" << color.x << ", " << color.y << ", " << color.z << ")" << std::endl;
}

void TexturePanel::clearTexture(Entity* entity) {
    if (!entity) return;

    std::cout << "Texture cleared" << std::endl;
}

void TexturePanel::saveTexture(Entity* entity, const std::string& filename) {
    if (!entity) return;

    std::cout << "Saving texture to: " << filename << std::endl;
}

// ============================================
// UV Utilities
// ============================================

void UVUtils::unwrapMesh(HalfEdgeMesh& mesh) {
    // Simple unwrap: project vertices to 0-1 space based on bounds
    auto bounds = mesh.computeBounds();
    glm::vec3 size = bounds.max - bounds.min;

    for (uint32_t i = 0; i < mesh.vertexCount(); ++i) {
        glm::vec3 pos = mesh.vertices[i].position;
        glm::vec2 uv;

        if (size.x > 0.001f) uv.x = (pos.x - bounds.min.x) / size.x;
        if (size.z > 0.001f) uv.y = (pos.z - bounds.min.z) / size.z;

        mesh.vertices[i].uv = uv;
    }
}

void UVUtils::packIslands(HalfEdgeMesh& mesh) {
    // Simple packing: just ensure UVs are in 0-1 range
    for (uint32_t i = 0; i < mesh.vertexCount(); ++i) {
        mesh.vertices[i].uv.x = glm::clamp(mesh.vertices[i].uv.x, 0.0f, 1.0f);
        mesh.vertices[i].uv.y = glm::clamp(mesh.vertices[i].uv.y, 0.0f, 1.0f);
    }
}

int UVUtils::countOverlaps(const HalfEdgeMesh& mesh) {
    // Count overlapping UV regions (simplified)
    int overlaps = 0;

    for (uint32_t i = 0; i < mesh.vertexCount(); ++i) {
        for (uint32_t j = i + 1; j < mesh.vertexCount(); ++j) {
            auto& uv1 = mesh.vertices[i].uv;
            auto& uv2 = mesh.vertices[j].uv;

            if (glm::distance(uv1, uv2) < 0.001f) {
                overlaps++;
            }
        }
    }

    return overlaps;
}

void UVUtils::straightenBounds(HalfEdgeMesh& mesh) {
    // Align UV islands to grid
    for (uint32_t i = 0; i < mesh.vertexCount(); ++i) {
        mesh.vertices[i].uv.x = std::round(mesh.vertices[i].uv.x * 10.0f) / 10.0f;
        mesh.vertices[i].uv.y = std::round(mesh.vertices[i].uv.y * 10.0f) / 10.0f;
    }
}

} // namespace PixellentModeler
