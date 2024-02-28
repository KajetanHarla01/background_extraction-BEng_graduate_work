#ifndef __AVS_CAM_PARAMS_H__
#define __AVS_CAM_PARAMS_H__

#include "TypeDef.h"
#include "avsMatrix.h"

class CamParams {
public:
	Matrix_4x4 m_mExtMat; //4x4
	Matrix_4x4 m_mIntMat; //4x4

	Matrix_4x4 m_mCvtdExtMat;
	Matrix_4x4 m_mProjMat;
	Matrix_4x4 m_mInvProjMat;

	~CamParams();
	CamParams();
	void calc();
	void calcCvtdExtMat();
	void calcProjMat();
	void calcInvProjMat();
};

#endif
