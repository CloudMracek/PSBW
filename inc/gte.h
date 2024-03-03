#pragma once

#ifdef __cplusplus
extern "C" {
#endif

void gte_setup_3d(int width, int height, int otSize);

void gte_rotate_current_matrix(int yaw, int pitch, int roll);

//static void gte_multiply_curent_matrix_by_vectors(GTEMatrix *output);

#ifdef __cplusplus
}
#endif
