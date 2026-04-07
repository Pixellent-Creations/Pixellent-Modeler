#pragma once

#include <glm/glm.hpp>
#include <vector>
#include <cstdint>

namespace PixellentModeler {

class Entity;

// UV Editor for viewing and manipulating UV coordinates
class UVEditorPanel {
public:
    void render(Entity* selectedEntity);

    // UV operations
    void markSeams(Entity* entity, const std::vector<uint32_t>& edgeIds);
    void unwrapUVs(Entity* entity);
    void optimizePacking(Entity* entity);

private:
    bool m_showEditor = true;
    glm::vec2 m_panOffset{0.0f};
    float m_zoomLevel = 1.0f;

    // Seam marking state
    std::vector<uint32_t> m_markedSeams;
};

// Texture Painter for painting on textures
class TexturePanel {
public:
    void render(Entity* selectedEntity);

    // Painting operations
    void paintColor(Entity* entity, const glm::vec3& color);
    void clearTexture(Entity* entity);
    void saveTexture(Entity* entity, const std::string& filename);

private:
    bool m_showPainter = true;
    glm::vec3 m_brushColor{1.0f, 1.0f, 1.0f};
    float m_brushSize = 16.0f;
    float m_brushIntensity = 1.0f;

    // Paint state
    bool m_isPainting = false;
};

// UV Utilities
namespace UVUtils {

// Unwrap UV coordinates from 3D mesh
void unwrapMesh(class HalfEdgeMesh& mesh);

// Pack UV islands into 0-1 space
void packIslands(class HalfEdgeMesh& mesh);

// Check for UV overlaps
int countOverlaps(const class HalfEdgeMesh& mesh);

// Straighten UV bounds
void straightenBounds(class HalfEdgeMesh& mesh);

} // namespace UVUtils

} // namespace PixellentModeler
