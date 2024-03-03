#include "psbw/Mesh.h"

#include <ps1/cop0gte.h>
#include <ps1/gpucmd.h>

#include "draw.h"
#include "vsync.h"
#include "gte.h"

typedef struct
{
    uint8_t vertices[4];
    uint32_t color;
} Face;

#define ONE (1 << 12)

Mesh::Mesh(){}

int i = 0;
void Mesh::execute(GameObject *parent)
{

    gte_setTranslationVector(0, 0, 256);
    gte_setRotationMatrix(
        ONE,    0,      0,
        0,      ONE,    0,
        0,      0,      ONE
    );
    gte_rotate_current_matrix((i*16)%4096,(i*16)%4096,0);
    i++;

    uint32_t *ptr;
    for (int i = 0; i < mesh->header->numFaces; i++) {
			const BWM_FACE *face = &mesh->faces[i];

			// Apply perspective projection to the first 3 vertices. The GTE can
			// only process up to 3 vertices at a time, so we'll transform the
			// last one separately.
			gte_loadV0((GTEVector16*) &mesh->vertices[face->v0]);
			gte_loadV1((GTEVector16*) &mesh->vertices[face->v1]);
			gte_loadV2((GTEVector16*) &mesh->vertices[face->v2]);
			gte_command(GTE_CMD_RTPT | GTE_SF);

			// Determine the winding order of the vertices on screen. If they
			// are ordered clockwise then the face is visible, otherwise it can
			// be skipped as it is not facing the camera.
			gte_command(GTE_CMD_NCLIP);

			if (gte_getMAC0() <= 0)
				continue;

			// Save the first transformed vertex (the GTE only keeps the X/Y
			// coordinates of the last 3 vertices processed and Z coordinates of
			// the last 4 vertices processed) and apply projection to the last
			// vertex.
			uint32_t xy0 = gte_getSXY0();

			gte_loadV0((GTEVector16*) &mesh->vertices[face->v3]);
			gte_command(GTE_CMD_RTPS | GTE_SF);

			// Calculate the average Z coordinate of all vertices and use it to
			// determine the ordering table bucket index for this face.
			gte_command(GTE_CMD_AVSZ4 | GTE_SF);
			int zIndex = gte_getOTZ();

			if ((zIndex < 0) || (zIndex >= getOtSize()))
				continue;

			// Create a new quad and give its vertices the X/Y coordinates
			// calculated by the GTE.
			ptr    = dma_get_chain_pointer(5, zIndex);
			ptr[0] = 0xffffff | gp0_shadedQuad(false, false, false);
			ptr[1] = xy0;
			gte_storeSXY012(&ptr[2]);
		}
}