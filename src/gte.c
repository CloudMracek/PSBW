#include "gte.h"

#include <ps1/cop0gte.h>

#include "trig.h"

#define ONE (1 << 12)

void gte_setup_3d(int width, int height, int otSize) {
	// enable coprocessor
	cop0_setSR(cop0_getSR() | COP0_SR_CU2);

	gte_setXYOrigin(width / 2, height / 2);
	gte_setFieldOfView(width);

	gte_setZScaleFactor(ONE / otSize);
}

void gte_multiply_curent_matrix_by_vectors(GTEMatrix *output) {
	// Multiply the GTE's current matrix by the matrix whose column vectors are
	// V0/V1/V2, then store the result to the provided location. This has to be
	// done one column at a time, as the GTE only supports multiplying a matrix
	// by a vector using the MVMVA command.
	gte_command(GTE_CMD_MVMVA | GTE_SF | GTE_MX_RT | GTE_V_V0 | GTE_CV_NONE);
	output->values[0][0] = gte_getIR1();
	output->values[1][0] = gte_getIR2();
	output->values[2][0] = gte_getIR3();

	gte_command(GTE_CMD_MVMVA | GTE_SF | GTE_MX_RT | GTE_V_V1 | GTE_CV_NONE);
	output->values[0][1] = gte_getIR1();
	output->values[1][1] = gte_getIR2();
	output->values[2][1] = gte_getIR3();

	gte_command(GTE_CMD_MVMVA | GTE_SF | GTE_MX_RT | GTE_V_V2 | GTE_CV_NONE);
	output->values[0][2] = gte_getIR1();
	output->values[1][2] = gte_getIR2();
	output->values[2][2] = gte_getIR3();
}

void gte_rotate_current_matrix(int yaw, int pitch, int roll) {
	static GTEMatrix multiplied;
	int              s, c;

	// For each axis, compute the rotation matrix then "combine" it with the
	// GTE's current matrix by multiplying the two and writing the result back
	// to the GTE's registers.
	if (yaw) {
		s = isin(yaw);
		c = icos(yaw);

		gte_setColumnVectors(
			c, -s,   0,
			s,  c,   0,
			0,  0, ONE
		);
		gte_multiply_curent_matrix_by_vectors(&multiplied);
		gte_loadRotationMatrix(&multiplied);
	}
	if (pitch) {
		s = isin(pitch);
		c = icos(pitch);

		gte_setColumnVectors(
			 c,   0, s,
			 0, ONE, 0,
			-s,   0, c
		);
		gte_multiply_curent_matrix_by_vectors(&multiplied);
		gte_loadRotationMatrix(&multiplied);
	}
	if (roll) {
		s = isin(roll);
		c = icos(roll);

		gte_setColumnVectors(
			ONE, 0,  0,
			  0, c, -s,
			  0, s,  c
		);
		gte_multiply_curent_matrix_by_vectors(&multiplied);
		gte_loadRotationMatrix(&multiplied);
	}
}
