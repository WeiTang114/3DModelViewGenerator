#ifndef __MESH__
#define __MESH__

typedef struct Vertex {
    float x, y, z;
} Vertex;

typedef struct Face {
    Face(void) : nverts(0), verts(0) {};
    int nverts;
    Vertex **verts;
    float normal[3];
} Face;

typedef struct Mesh {
    Mesh(void) : nverts(0), verts(0), nfaces(0), faces(0) {};
    int nverts;
    Vertex *verts;
    int nfaces;
    Face *faces;
} Mesh;

#endif
