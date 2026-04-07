#pragma once

namespace PixellentModeler {

class HalfEdgeMesh;

enum class UVProjection { Planar, Box, Cylindrical, Spherical };

namespace UVMapping {
    void projectPlanar(HalfEdgeMesh& mesh, int axis = 1);  // 0=X, 1=Y, 2=Z - project from that axis
    void projectBox(HalfEdgeMesh& mesh);  // project each face from its dominant axis
    void projectCylindrical(HalfEdgeMesh& mesh);
    void projectSpherical(HalfEdgeMesh& mesh);
}

} // namespace PixellentModeler
