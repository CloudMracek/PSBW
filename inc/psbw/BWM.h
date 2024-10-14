#pragma once

#include <stdint.h>

typedef struct [[gnu::packed]] BWM_HEADER {
    char magic[3];
    uint8_t version;
    uint16_t numFaces, numVertices, numNormals, numUVs;
};

typedef struct BWM_VERTEX {
    int16_t x,y,z,pad;
};

typedef struct [[gnu::packed]] BWM_NORMAL {
    int16_t x,y,z;
};

typedef struct [[gnu::packed]] BWM_UV {
    uint8_t u,v;
};

typedef struct [[gnu::packed]] BWM_FACE{
    uint8_t vertexCount;
    uint16_t v0, v1, v2, v3, n0, n1, n2, n3, u0, u1, u2, u3;
};

class BWM {
    public:
        BWM_HEADER* header;
        BWM_VERTEX* vertices;
        BWM_NORMAL* normals;
        BWM_UV* uvs;
        BWM_FACE* faces;
};
