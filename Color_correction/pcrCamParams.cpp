#include "TypeDef.h"
#include "pcrCamParams.h"

CamParams::CamParams() {

	m_mExtMat.init();
	m_mIntMat.init();
	m_mCvtdExtMat.init();

	return;
}

void CamParams::calcCvtdExtMat() {

	Matrix_4x4 R;
	Matrix_3x1 T;

	for (UInt h = 0; h < 3; h++) {
		for (UInt w = 0; w < 3; w++) {
			m_mCvtdExtMat.at(h, w) = m_mExtMat.at(h, w);
			R.at(h, w) = m_mExtMat.at(h, w);
		}
		T.at(h) = m_mExtMat.at(h, 3);
	}

	Matrix_3x1 mRT;
	matrixMultiply_4x4_3x1(R, T, mRT);

	for (UInt w = 0; w < 3; w++) m_mCvtdExtMat.at(3, w) = 0;
	for (UInt h = 0; h < 3; h++) m_mCvtdExtMat.at(h, 3) = -mRT.at(h);
	m_mCvtdExtMat.at(3, 3) = 1;

	return;
}

void CamParams::calcProjMat() {

	matrixMultiply_4x4_4x4(m_mIntMat, m_mCvtdExtMat, m_mProjMat);

	return;
}

void CamParams::calcInvProjMat() {

	matrixInversion_4x4(m_mProjMat, m_mInvProjMat);

	return;
}

void CamParams::calc() {

	calcCvtdExtMat();
	calcProjMat();
	calcInvProjMat();

	return;
}
