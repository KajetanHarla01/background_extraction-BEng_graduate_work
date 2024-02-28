#include "pcrMatrix.h"

Matrix_4x4::Matrix_4x4() {
	init();
	return;
}

Matrix_3x1::Matrix_3x1() {
	init();
	return;
}

Void Matrix_4x4::init() {
		
	for (UInt h = 0;h < 4;h++) {
		for (UInt w = 0;w < 4;w++) {
			if (h == w) m_adValues[h][w] = 1;
			else m_adValues[h][w] = 0;
		}
	}

	return;
}

Void Matrix_3x1::init() {

	for (UInt h = 0;h < 3;h++) {
		m_adValues[h] = 0;
	}

	return;
}

Double& Matrix_4x4::at(UInt h, UInt w) {

	return m_adValues[h][w];
}

Double& Matrix_3x1::at(UInt h) {

	return m_adValues[h];
}

Void matrixInversion_4x4(Matrix_4x4 &A, Matrix_4x4 &invA) {

	Double m[16], inv[16], det;

	for (int h = 0, pp = 0; h < 4; h++) {
		for (int w = 0; w < 4; w++, pp++) {
			m[pp] = A.at(h, w);
		}
	}

	inv[0] = m[5] * m[10] * m[15] - m[5] * m[11] * m[14] - m[9] * m[6] * m[15] + m[9] * m[7] * m[14] + m[13] * m[6] * m[11] - m[13] * m[7] * m[10];
	inv[1] = -m[1] * m[10] * m[15] + m[1] * m[11] * m[14] + m[9] * m[2] * m[15] - m[9] * m[3] * m[14] - m[13] * m[2] * m[11] + m[13] * m[3] * m[10];
	inv[2] = m[1] * m[6] * m[15] - m[1] * m[7] * m[14] - m[5] * m[2] * m[15] + m[5] * m[3] * m[14] + m[13] * m[2] * m[7] - m[13] * m[3] * m[6];
	inv[3] = -m[1] * m[6] * m[11] + m[1] * m[7] * m[10] + m[5] * m[2] * m[11] - m[5] * m[3] * m[10] - m[9] * m[2] * m[7] + m[9] * m[3] * m[6];
	inv[4] = -m[4] * m[10] * m[15] + m[4] * m[11] * m[14] + m[8] * m[6] * m[15] - m[8] * m[7] * m[14] - m[12] * m[6] * m[11] + m[12] * m[7] * m[10];
	inv[5] = m[0] * m[10] * m[15] - m[0] * m[11] * m[14] - m[8] * m[2] * m[15] + m[8] * m[3] * m[14] + m[12] * m[2] * m[11] - m[12] * m[3] * m[10];
	inv[6] = -m[0] * m[6] * m[15] + m[0] * m[7] * m[14] + m[4] * m[2] * m[15] - m[4] * m[3] * m[14] - m[12] * m[2] * m[7] + m[12] * m[3] * m[6];
	inv[7] = m[0] * m[6] * m[11] - m[0] * m[7] * m[10] - m[4] * m[2] * m[11] + m[4] * m[3] * m[10] + m[8] * m[2] * m[7] - m[8] * m[3] * m[6];
	inv[8] = m[4] * m[9] * m[15] - m[4] * m[11] * m[13] - m[8] * m[5] * m[15] + m[8] * m[7] * m[13] + m[12] * m[5] * m[11] - m[12] * m[7] * m[9];
	inv[9] = -m[0] * m[9] * m[15] + m[0] * m[11] * m[13] + m[8] * m[1] * m[15] - m[8] * m[3] * m[13] - m[12] * m[1] * m[11] + m[12] * m[3] * m[9];
	inv[10] = m[0] * m[5] * m[15] - m[0] * m[7] * m[13] - m[4] * m[1] * m[15] + m[4] * m[3] * m[13] + m[12] * m[1] * m[7] - m[12] * m[3] * m[5];
	inv[11] = -m[0] * m[5] * m[11] + m[0] * m[7] * m[9] + m[4] * m[1] * m[11] - m[4] * m[3] * m[9] - m[8] * m[1] * m[7] + m[8] * m[3] * m[5];
	inv[12] = -m[4] * m[9] * m[14] + m[4] * m[10] * m[13] + m[8] * m[5] * m[14] - m[8] * m[6] * m[13] - m[12] * m[5] * m[10] + m[12] * m[6] * m[9];
	inv[13] = m[0] * m[9] * m[14] - m[0] * m[10] * m[13] - m[8] * m[1] * m[14] + m[8] * m[2] * m[13] + m[12] * m[1] * m[10] - m[12] * m[2] * m[9];
	inv[14] = -m[0] * m[5] * m[14] + m[0] * m[6] * m[13] + m[4] * m[1] * m[14] - m[4] * m[2] * m[13] - m[12] * m[1] * m[6] + m[12] * m[2] * m[5];
	inv[15] = m[0] * m[5] * m[10] - m[0] * m[6] * m[9] - m[4] * m[1] * m[10] + m[4] * m[2] * m[9] + m[8] * m[1] * m[6] - m[8] * m[2] * m[5];

	det = m[0] * inv[0] + m[1] * inv[4] + m[2] * inv[8] + m[3] * inv[12];
	det = 1.0 / det;

	for (int h = 0, pp = 0; h < 4; h++) {
		for (int w = 0; w < 4; w++, pp++) {
			invA.at(h, w) = inv[pp] * det;
		}
	}

	return;
}

Void matrixMultiply_4x4_4x4(Matrix_4x4 &A, Matrix_4x4 &B, Matrix_4x4 &C) {

	for (UInt i = 0; i < 4; i++) {
		for (UInt j = 0; j < 4; j++) {
			C.at(i, j) = 0;
			for (UInt k = 0; k < 4; k++) {
				C.at(i, j) += (A.at(i, k) * B.at(k, j));
			}
		}
	}

	return;
}

Void matrixMultiply_4x4_3x1(Matrix_4x4 &A, Matrix_3x1 &B, Matrix_3x1 &C) {

	for (UInt h = 0;h < 3;h++) {
		C.at(h) = A.at(h, 0)*B.at(0) + A.at(h, 1)*B.at(1) + A.at(h, 2)*B.at(2) + A.at(h, 3);
	}

	return;
}
