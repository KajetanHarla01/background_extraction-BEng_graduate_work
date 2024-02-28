#ifndef __PCR_CAM_PARAMS_H__
#define __PCR_CAM_PARAMS_H__

#include "TypeDef.h"
#include "pcrMatrix.h"

class CamParams {
public:
	Matrix_4x4 m_mExtMat;
	Matrix_4x4 m_mIntMat;

	Matrix_4x4 m_mCvtdExtMat;
	Matrix_4x4 m_mProjMat;
	Matrix_4x4 m_mInvProjMat;

	CamParams();
	void calc();
	void calcCvtdExtMat();
	void calcProjMat();
	void calcInvProjMat();
};

#endif
