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

#define NUM_CUBE_VERTICES 8
#define NUM_CUBE_FACES 6

static const GTEVector16 cubeVertices[NUM_CUBE_VERTICES] = {
    {.x = -32, .y = -32, .z = -32},
    {.x = 32, .y = -32, .z = -32},
    {.x = -32, .y = 32, .z = -32},
    {.x = 32, .y = 32, .z = -32},
    {.x = -32, .y = -32, .z = 32},
    {.x = 32, .y = -32, .z = 32},
    {.x = -32, .y = 32, .z = 32},
    {.x = 32, .y = 32, .z = 32}};

// Note that there are several requirements on the order of vertices:
// - they must be arranged in a Z-like shape rather than clockwise or
//   counterclockwise, since the GPU processes a quad with vertices (A, B, C, D)
//   as two triangles with vertices (A, B, C) and (B, C, D) respectively;
// - the first 3 vertices must be ordered clockwise when the face is viewed from
//   the front, as the code relies on this to determine whether or not the quad
//   is facing the camera (see main()).
// For instance, only the first of these faces (viewed from the front) has its
// vertices ordered correctly:
//     0----1        0----1        2----3
//     |  / |        | \/ |        | \  |
//     | /  |        | /\ |        |  \ |
//     2----3        3----2        0----1
//     Correct    Not Z-shaped  Not clockwise
static const Face cubeFaces[NUM_CUBE_FACES] = {
    {.vertices = {0, 1, 2, 3}, .color = 0x0000ff},
    {.vertices = {6, 7, 4, 5}, .color = 0x00ff00},
    {.vertices = {4, 5, 0, 1}, .color = 0x00ffff},
    {.vertices = {7, 6, 3, 2}, .color = 0xff0000},
    {.vertices = {6, 4, 2, 0}, .color = 0xff00ff},
    {.vertices = {5, 7, 1, 3}, .color = 0xffff00}};

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
    for (int i = 0; i < NUM_CUBE_FACES; i++) {
			const Face *face = &cubeFaces[i];

			// Apply perspective projection to the first 3 vertices. The GTE can
			// only process up to 3 vertices at a time, so we'll transform the
			// last one separately.
			gte_loadV0(&cubeVertices[face->vertices[0]]);
			gte_loadV1(&cubeVertices[face->vertices[1]]);
			gte_loadV2(&cubeVertices[face->vertices[2]]);
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

			gte_loadV0(&cubeVertices[face->vertices[3]]);
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
			ptr[0] = face->color | gp0_shadedQuad(false, false, false);
			ptr[1] = xy0;
			gte_storeSXY012(&ptr[2]);
		}
}