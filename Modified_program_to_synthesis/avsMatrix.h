#ifndef __AVS_MATRIX_H__
#define __AVS_MATRIX_H__

#include "TypeDef.h"

class Matrix_3x1 {
public:
	Double m_adValues[3];

	Matrix_3x1();
	Void init();
	Double& at(UInt h);

	Matrix_3x1 &operator =(const Matrix_3x1 B) {

		for (UInt h = 0;h < 3;h++) {
			m_adValues[h] = B.m_adValues[h];
		}

		return *this;
	}
};

class Matrix_4x4 {
public:
	Double m_adValues[4][4];

	Matrix_4x4();
	Void init();
	Double& at(UInt h, UInt w);

	Matrix_4x4 &operator =(const Matrix_4x4 B) {
		
		for (UInt h = 0;h < 4;h++) {
			for (UInt w = 0;w < 4;w++) {
				m_adValues[h][w] = B.m_adValues[h][w];
			}
		}

		return *this;
	}
};

Void matrixMultiply_4x4_4x4(Matrix_4x4 &A, Matrix_4x4 &B, Matrix_4x4 &C);

Void matrixMultiply_4x4_3x1(Matrix_4x4 &A, Matrix_3x1 &B, Matrix_3x1 &C);

Void matrixInversion_4x4(Matrix_4x4 &A, Matrix_4x4 &invA);

#endif
