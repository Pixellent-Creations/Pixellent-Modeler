#pragma once

#include <memory>

#include "PixellentModeler/Mesh/HalfEdgeMesh.hpp"

namespace PixellentModeler {
namespace MeshPrimitives {

std::unique_ptr<HalfEdgeMesh> createCube(float size = 1.0f);
std::unique_ptr<HalfEdgeMesh> createPlane(float size = 1.0f, int subdivisions = 1);
std::unique_ptr<HalfEdgeMesh> createUVSphere(float radius = 0.5f, int segments = 32, int rings = 16);
std::unique_ptr<HalfEdgeMesh> createCylinder(float radius = 0.5f, float height = 1.0f, int segments = 32);
std::unique_ptr<HalfEdgeMesh> createCone(float radius = 0.5f, float height = 1.0f, int segments = 32);
std::unique_ptr<HalfEdgeMesh> createTorus(float majorRadius = 0.5f, float minorRadius = 0.2f,
                                          int majorSegments = 32, int minorSegments = 16);

} // namespace MeshPrimitives
} // namespace PixellentModeler
