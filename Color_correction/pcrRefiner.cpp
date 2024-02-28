#include "pcrRefiner.h"

Refiner::~Refiner() {

	return;
}

Refiner::Refiner(cArray<YUV*> yuvIn, cArray<YUV*> yuvOut, Config *cfg) {

	m_uiNumberOfInputViews = yuvIn.size();
	m_uiNumberOfOutputViews = yuvOut.size();

	m_yuvInput = yuvIn;
	m_yuvOutput = yuvOut;

	for (UInt o = 0;o < m_uiNumberOfOutputViews;o++) {
		cArray<YUV*> inter2;
		for (UInt i = 0;i < m_uiNumberOfInputViews;i++) {
			YUV *inter = new YUV(m_yuvOutput[o]->m_iWidth, m_yuvOutput[o]->m_iHeight);
			inter2.push_back(inter);
		}
		m_yuvIntermediate.push_back(inter2);
	}

	m_uiSynthesisMethod = cfg->m_uiSynthesisMethod;

	Float fInvMaxDepth = 1.0 / ((1 << cfg->m_uiDepthBitsPerSample) - 1);
	m_fDepthBlendingThreshold = cfg->m_fDepthBlendingThreshold * fInvMaxDepth * (1.0 / cfg->m_dZNear - 1.0 / cfg->m_dZFar);

	m_dWeightIIR = WEIGHT_IIR;

	m_IIR = new cArray<Double>[m_uiNumberOfInputViews];

	Int BW = m_yuvInput[0]->m_iWidth / BACKGROUND_WH_STEP;
	Int BH = m_yuvInput[0]->m_iHeight / BACKGROUND_WH_STEP;

	m_yuvBackground = new Double **[m_uiNumberOfInputViews];
	for (Int i = 0;i < m_uiNumberOfInputViews;i++) {
		m_yuvBackground[i] = new Double *[3];
		for (Int c = 0;c < 3;c++) {
			m_yuvBackground[i][c] = new Double[BW * BH];
		}
	}

	return;
}

void Refiner::cvtInputArraysToProcessing() {

	for (UInt i = 0;i < m_uiNumberOfInputViews;i++) {
		m_yuvInput[i]->cvtInputArraysToProcessing();
	}

	return;
}

void Refiner::cvtProcessingArraysToOutput() {

	for (UInt o = 0;o < m_uiNumberOfOutputViews;o++) {
		m_yuvOutput[o]->cvtProcessingArraysToOutput();
	}

	return;
}

void Refiner::clearProcessingArrays() {

	for (UInt o = 0;o < m_uiNumberOfOutputViews;o++) {
		for (UInt c = 0;c < 4;c++) memset(m_yuvOutput[o]->m_afYUVD[c], 0, m_yuvOutput[o]->m_uiDepthLumaFrameSizeInPixels * 4);
		for (UInt i = 0;i < m_uiNumberOfInputViews;i++) {
			for (UInt c = 0;c < 4;c++) memset(m_yuvIntermediate[o][i]->m_afYUVD[c], 0, m_yuvIntermediate[o][i]->m_uiDepthLumaFrameSizeInPixels * 4);
		}
	}

	return;
}

void Refiner::calcImPosPersp(Double &wPos, Double &hPos, Double &invTargetZ, Matrix_3x1 *XYZ, Matrix_4x4 *mat) {

	Matrix_3x1 tmpMat;
	matrixMultiply_4x4_3x1(*mat, *XYZ, tmpMat);

	if (tmpMat.at(2) <= 0) return;

	invTargetZ = 1 / tmpMat.at(2);

	wPos = tmpMat.at(0) * invTargetZ;
	hPos = tmpMat.at(1) * invTargetZ;

	return;
}

void Refiner::calcImPosOmni(Int &W, Int &H, Double &wPos, Double &hPos, Double &invTargetZ, Matrix_3x1 *XYZ, Double &OAovL, Double &OAovW, Double &OAovB, Double &OAovH) {

	Double X = XYZ->at(0);
	Double Y = XYZ->at(1);
	Double Z = XYZ->at(2);

	invTargetZ = 1.0 / sqrt(X * X + Y * Y + Z * Z);

	Double phi = atan2(Z, X);
	Double theta = asin(Y * invTargetZ);

	wPos = (phi - OAovL) * (W - 1) / OAovW;
	hPos = (theta - OAovB) * (H - 1) / OAovH;

	return;
}

void Refiner::calcXYZPersp(Int &w, Int &h, Double &invZ, Matrix_3x1 *XYZ, Matrix_4x4 *mat) {

	Matrix_3x1 whz;
	Double z = 1.0 / invZ;
	whz.at(0) = w * z;
	whz.at(1) = h * z;
	whz.at(2) = 1 * z;

	matrixMultiply_4x4_3x1(*mat, whz, *XYZ);

	return;
}

void Refiner::projectDepth(UInt i, UInt o, UInt inFormat, UInt outFormat) {

	Int IW = m_yuvInput[i]->m_iWidth;
	Int IH = m_yuvInput[i]->m_iHeight;

	Int OW = m_yuvIntermediate[o][i]->m_iWidth;
	Int OH = m_yuvIntermediate[o][i]->m_iHeight;

	Double IAovL = m_yuvInput[i]->m_dAovL;
	Double IAovW = m_yuvInput[i]->m_dAovW;
	Double IAovB = m_yuvInput[i]->m_dAovB;
	Double IAovH = m_yuvInput[i]->m_dAovH;

	Double OAovL = m_yuvOutput[o]->m_dAovL;
	Double OAovW = m_yuvOutput[o]->m_dAovW;
	Double OAovB = m_yuvOutput[o]->m_dAovB;
	Double OAovH = m_yuvOutput[o]->m_dAovH;

	Double **LUT_Theta = new Double*[2];
	Double **LUT_Phi = new Double*[2];

	for (UInt i = 0;i < 2;i++) LUT_Theta[i] = new Double[IH];
	for (UInt i = 0;i < 2;i++) LUT_Phi[i] = new Double[IW];

	for (Int h = 0;h < IH;h++) {
		Double theta = Double(h) * IAovH / (IH - 1) + IAovB;
		LUT_Theta[0][h] = cos(theta);
		LUT_Theta[1][h] = sin(theta);
	}
	for (Int w = 0;w < IW;w++) {
		Double phi = Double(w) * IAovW / (IW - 1) + IAovL;
		LUT_Phi[0][w] = cos(phi);
		LUT_Phi[1][w] = sin(phi);
	}

	Matrix_3x1 *XYZ = new Matrix_3x1;
	Matrix_3x1 *XYZ2 = new Matrix_3x1;

	Matrix_4x4 R1;
	Matrix_4x4 R2;
	for (UInt h = 0;h < 3;h++) {
		for (UInt w = 0;w < 3;w++) R2.at(h, w) = m_yuvInput[i]->m_currentCamParams->m_mCvtdExtMat.at(h, w);
	}
	matrixInversion_4x4(R2, R1);
	for (UInt h = 0;h < 3;h++) {
		for (UInt w = 0;w < 3;w++) R2.at(h, w) = m_yuvOutput[o]->m_currentCamParams->m_mCvtdExtMat.at(h, w);
	}

	Double wPos, hPos, dInvZ, dInvTargetZ;

	for (Int h = 0;h < IH;h++) {
		Int hW = h * IW;
		for (Int w = 0;w < IW;w++) {
			Int pp = hW + w;

			dInvZ = m_yuvInput[i]->m_afYUVD[3][pp];
			if (dInvZ == 0) continue;

			if (inFormat == 0)
				calcXYZPersp(w, h, dInvZ, XYZ, &(m_yuvInput[i]->m_currentCamParams->m_mInvProjMat));
			else {
				Double z = 1.0 / dInvZ;

				XYZ2->at(0) = z * LUT_Theta[0][h] * LUT_Phi[0][w];
				XYZ2->at(1) = z * LUT_Theta[1][h];
				XYZ2->at(2) = z * LUT_Theta[0][h] * LUT_Phi[1][w];

				matrixMultiply_4x4_3x1(R1, *XYZ2, *XYZ);
				XYZ->at(0) += m_yuvInput[i]->m_currentCamParams->m_mExtMat.at(0, 3);
				XYZ->at(1) += m_yuvInput[i]->m_currentCamParams->m_mExtMat.at(1, 3);
				XYZ->at(2) += m_yuvInput[i]->m_currentCamParams->m_mExtMat.at(2, 3);
			}

			if (outFormat == 0)
				calcImPosPersp(wPos, hPos, dInvTargetZ, XYZ, &(m_yuvOutput[o]->m_currentCamParams->m_mProjMat));
			else {
				XYZ->at(0) -= m_yuvOutput[o]->m_currentCamParams->m_mExtMat.at(0, 3);
				XYZ->at(1) -= m_yuvOutput[o]->m_currentCamParams->m_mExtMat.at(1, 3);
				XYZ->at(2) -= m_yuvOutput[o]->m_currentCamParams->m_mExtMat.at(2, 3);

				matrixMultiply_4x4_3x1(R2, *XYZ, *XYZ2);

				calcImPosOmni(OW, OH, wPos, hPos, dInvTargetZ, XYZ2, OAovL, OAovW, OAovB, OAovH);
			}

			if (wPos >= 0 && wPos <= OW - 1 && hPos >= 0 && hPos <= OH - 1) {

				Int ppt = Int(hPos + 0.5)*OW + Int(wPos + 0.5);

				if (m_yuvIntermediate[o][i]->m_afYUVD[3][ppt] < dInvTargetZ) {
					m_yuvIntermediate[o][i]->m_afYUVD[3][ppt] = dInvTargetZ;
				}

			}

		}//w
	}//h

	delete LUT_Phi[0];
	delete LUT_Phi[1];
	delete LUT_Phi;

	delete LUT_Theta[0];
	delete LUT_Theta[1];
	delete LUT_Theta;

	delete XYZ;
	delete XYZ2;

	return;
}

void Refiner::filterDepth(UInt i, UInt o) {
	Int holeSize = 2;

	Float tmpVal[4];
	Float *tmpArr = new Float[m_yuvOutput[o]->m_uiDepthLumaFrameSizeInPixels];

	Int H = m_yuvIntermediate[o][i]->m_iHeight;
	Int W = m_yuvIntermediate[o][i]->m_iWidth;

	for (Int m = 1;m <= holeSize;m++) {
		for (Int h = m;h < H - m;h++) {
			Int hW = h * W;
			Int mW = m * W;
			for (Int w = m;w < W - m;w++) {
				Int pp = hW + w;

				for (Int v = 0;v < 4;v++) tmpVal[v] = 0;

				if (m_yuvIntermediate[o][i]->m_afYUVD[3][pp - m] != 0 && m_yuvIntermediate[o][i]->m_afYUVD[3][pp + m] != 0) {
					if (m_yuvIntermediate[o][i]->m_afYUVD[3][pp] == 0 || m_yuvIntermediate[o][i]->m_afYUVD[3][pp] < Min(m_yuvIntermediate[o][i]->m_afYUVD[3][pp - m], m_yuvIntermediate[o][i]->m_afYUVD[3][pp + m])) {
						tmpVal[0] = Min(m_yuvIntermediate[o][i]->m_afYUVD[3][pp - m], m_yuvIntermediate[o][i]->m_afYUVD[3][pp + m]);
					}
				}
				if (m_yuvIntermediate[o][i]->m_afYUVD[3][pp - mW] != 0 && m_yuvIntermediate[o][i]->m_afYUVD[3][pp + mW] != 0) {
					if (m_yuvIntermediate[o][i]->m_afYUVD[3][pp] == 0 || m_yuvIntermediate[o][i]->m_afYUVD[3][pp] < Min(m_yuvIntermediate[o][i]->m_afYUVD[3][pp - mW], m_yuvIntermediate[o][i]->m_afYUVD[3][pp + mW])) {
						tmpVal[1] = Min(m_yuvIntermediate[o][i]->m_afYUVD[3][pp - mW], m_yuvIntermediate[o][i]->m_afYUVD[3][pp + mW]);
					}
				}
				if (m_yuvIntermediate[o][i]->m_afYUVD[3][pp - m - mW] != 0 && m_yuvIntermediate[o][i]->m_afYUVD[3][pp + m + mW] != 0) {
					if (m_yuvIntermediate[o][i]->m_afYUVD[3][pp] == 0 || m_yuvIntermediate[o][i]->m_afYUVD[3][pp] < Min(m_yuvIntermediate[o][i]->m_afYUVD[3][pp - m - mW], m_yuvIntermediate[o][i]->m_afYUVD[3][pp + m + mW])) {
						tmpVal[2] = Min(m_yuvIntermediate[o][i]->m_afYUVD[3][pp - m - mW], m_yuvIntermediate[o][i]->m_afYUVD[3][pp + m + mW]);
					}
				}
				if (m_yuvIntermediate[o][i]->m_afYUVD[3][pp - m + mW] != 0 && m_yuvIntermediate[o][i]->m_afYUVD[3][pp + m - mW] != 0) {
					if (m_yuvIntermediate[o][i]->m_afYUVD[3][pp] == 0 || m_yuvIntermediate[o][i]->m_afYUVD[3][pp] < Min(m_yuvIntermediate[o][i]->m_afYUVD[3][pp - m + mW], m_yuvIntermediate[o][i]->m_afYUVD[3][pp + m - mW])) {
						tmpVal[3] = Min(m_yuvIntermediate[o][i]->m_afYUVD[3][pp - m + mW], m_yuvIntermediate[o][i]->m_afYUVD[3][pp + m - mW]);
					}
				}

				Float minVal = BIG_FLOAT_VALUE;
				Int tmpCnt = 0;
				for (Int v = 0;v < 4;v++) {
					if (!tmpVal[v]) continue;
					if (minVal > tmpVal[v]) minVal = tmpVal[v];
					tmpCnt++;
				}
				if (tmpCnt) tmpArr[pp] = minVal;
				else tmpArr[pp] = 0;

			}//w
		}//h

		for (Int h = m;h < H - m;h++) {
			Int hW = h * W;
			for (Int w = m;w < W - m;w++) {
				Int pp = hW + w;

				if (tmpArr[pp] == 0) continue;
				m_yuvIntermediate[o][i]->m_afYUVD[3][pp] = tmpArr[pp];
				tmpArr[pp] = 0;

			}//w
		}//h
	}//m

	delete tmpArr;

	return;
}

void Refiner::projectTexture(UInt i, UInt o, UInt inFormat, UInt outFormat) {

	Int IW = m_yuvInput[i]->m_iWidth;
	Int IH = m_yuvInput[i]->m_iHeight;

	Int OW = m_yuvIntermediate[o][i]->m_iWidth;
	Int OH = m_yuvIntermediate[o][i]->m_iHeight;

	Double IAovL = m_yuvInput[i]->m_dAovL;
	Double IAovW = m_yuvInput[i]->m_dAovW;
	Double IAovB = m_yuvInput[i]->m_dAovB;
	Double IAovH = m_yuvInput[i]->m_dAovH;

	Double OAovL = m_yuvOutput[o]->m_dAovL;
	Double OAovW = m_yuvOutput[o]->m_dAovW;
	Double OAovB = m_yuvOutput[o]->m_dAovB;
	Double OAovH = m_yuvOutput[o]->m_dAovH;

	Double **LUT_Theta = new Double*[2];
	Double **LUT_Phi = new Double*[2];

	for (UInt i = 0;i < 2;i++) LUT_Theta[i] = new Double[OH];
	for (UInt i = 0;i < 2;i++) LUT_Phi[i] = new Double[OW];

	for (UInt h = 0;h < OH;h++) {
		Double theta = Double(h) * OAovH / (OH - 1) + OAovB;
		LUT_Theta[0][h] = cos(theta);
		LUT_Theta[1][h] = sin(theta);
	}
	for (UInt w = 0;w < OW;w++) {
		Double phi = Double(w) * OAovW / (OW - 1) + OAovL;
		LUT_Phi[0][w] = cos(phi);
		LUT_Phi[1][w] = sin(phi);
	}

	Matrix_3x1 *XYZ = new Matrix_3x1;
	Matrix_3x1 *XYZ2 = new Matrix_3x1;

	Double wPos, hPos, dInvZ, dInvTargetZ;

	Matrix_4x4 R1;
	Matrix_4x4 R2;
	for (UInt h = 0;h < 3;h++) {
		for (UInt w = 0;w < 3;w++) R2.at(h, w) = m_yuvOutput[o]->m_currentCamParams->m_mCvtdExtMat.at(h, w);
	}
	matrixInversion_4x4(R2, R1);
	for (UInt h = 0;h < 3;h++) {
		for (UInt w = 0;w < 3;w++) R2.at(h, w) = m_yuvInput[i]->m_currentCamParams->m_mCvtdExtMat.at(h, w);
	}

	for (Int h = 0;h < OH;h++) {
		Int hW = h * OW;
		for (Int w = 0;w < OW;w++) {
			Int pp = hW + w;

			dInvZ = m_yuvIntermediate[o][i]->m_afYUVD[3][pp];
			if (dInvZ == 0) continue;

			if (outFormat == 0)
				calcXYZPersp(w, h, dInvZ, XYZ, &(m_yuvOutput[o]->m_currentCamParams->m_mInvProjMat));
			else {
				Double z = 1.0 / dInvZ;

				XYZ2->at(0) = z * LUT_Theta[0][h] * LUT_Phi[0][w];
				XYZ2->at(1) = z * LUT_Theta[1][h];
				XYZ2->at(2) = z * LUT_Theta[0][h] * LUT_Phi[1][w];

				matrixMultiply_4x4_3x1(R1, *XYZ2, *XYZ);
				XYZ->at(0) += m_yuvOutput[o]->m_currentCamParams->m_mExtMat.at(0, 3);
				XYZ->at(1) += m_yuvOutput[o]->m_currentCamParams->m_mExtMat.at(1, 3);
				XYZ->at(2) += m_yuvOutput[o]->m_currentCamParams->m_mExtMat.at(2, 3);
			}

			if (inFormat == 0)
				calcImPosPersp(wPos, hPos, dInvTargetZ, XYZ, &(m_yuvInput[i]->m_currentCamParams->m_mProjMat));
			else {
				XYZ->at(0) -= m_yuvInput[i]->m_currentCamParams->m_mExtMat.at(0, 3);
				XYZ->at(1) -= m_yuvInput[i]->m_currentCamParams->m_mExtMat.at(1, 3);
				XYZ->at(2) -= m_yuvInput[i]->m_currentCamParams->m_mExtMat.at(2, 3);
				matrixMultiply_4x4_3x1(R2, *XYZ, *XYZ2);
				calcImPosOmni(IW, IH, wPos, hPos, dInvTargetZ, XYZ2, IAovL, IAovW, IAovB, IAovH);
			}

			copyColorFromInput(i, o, pp, wPos, hPos);

		}//w
	}//h

	delete LUT_Phi[0];
	delete LUT_Phi[1];
	delete LUT_Phi;

	delete LUT_Theta[0];
	delete LUT_Theta[1];
	delete LUT_Theta;

	delete XYZ;
	delete XYZ2;

	return;
}

void Refiner::copyColorFromInput(UInt &i, UInt &o, Int &pp, Double &wPos, Double &hPos) {

	if (wPos >= 0 && wPos < m_yuvInput[i]->m_iWidth - 1 && hPos >= 0 && hPos < m_yuvInput[i]->m_iHeight - 1) {
		UInt ppt = Int(hPos + .5)*m_yuvInput[i]->m_iWidth + Int(wPos + .5);
		if (m_yuvInput[i]->m_afYUVD[3][ppt] == 0) {
			m_yuvIntermediate[o][i]->m_afYUVD[3][pp] = 0;
			return;
		}

		m_yuvIntermediate[o][i]->m_afYUVD[0][pp] = m_yuvInput[i]->m_afYUVD[0][ppt];
		m_yuvIntermediate[o][i]->m_afYUVD[1][pp] = m_yuvInput[i]->m_afYUVD[1][ppt];
		m_yuvIntermediate[o][i]->m_afYUVD[2][pp] = m_yuvInput[i]->m_afYUVD[2][ppt];
	}
	else {
		m_yuvIntermediate[o][i]->m_afYUVD[3][pp] = 0; //clear depth info where no texture was projected
	}

	return;
}

void Refiner::loadInputDepthMaps(UInt frame) {

	for (UInt i = 0;i < m_uiNumberOfInputViews;i++) {
		m_yuvInput[i]->readDepthFrame(frame);
	}

	return;
}

void Refiner::loadInputViews(UInt frame) {

	for (UInt i = 0;i < m_uiNumberOfInputViews;i++) {
		m_yuvInput[i]->readViewFrame(frame);
	}

	return;
}

void Refiner::saveOutputDepthMaps(Bool append) {

	for (UInt o = 0;o < m_uiNumberOfOutputViews;o++) {
		m_yuvOutput[o][0].writeDepthFrame(append);
	}

	return;
}

void Refiner::saveOutputViews(Bool append) {

	for (UInt o = 0;o < m_uiNumberOfOutputViews;o++) {
		m_yuvOutput[o][0].writeViewFrame(append);
	}

	return;
}

void Refiner::inpaintDisocclusions8(UInt o) {

	inpaintSmallAreas(o);

	Int **inpaintingLUT = new Int*[8];
	for (UInt d = 0;d < 8;d++) inpaintingLUT[d] = new Int[m_yuvOutput[o]->m_uiDepthLumaFrameSizeInPixels];

	Int H = m_yuvOutput[o]->m_iHeight;
	Int W = m_yuvOutput[o]->m_iWidth;

	//analysis from top-left

	for (Int h = 0;h < H;h++) {
		Int hW = h * W;
		for (Int w = 0;w < W;w++) {
			Int pp = hW + w;

			inpaintingLUT[0][pp] = pp;
			inpaintingLUT[1][pp] = pp;
			inpaintingLUT[2][pp] = pp;
			inpaintingLUT[3][pp] = pp;

			if (m_yuvOutput[o][0].m_afYUVD[3][pp] == 0) {

				if (w > 0 && h > 0) inpaintingLUT[0][pp] = inpaintingLUT[0][pp - 1 - W];
				else inpaintingLUT[0][pp] = -1;

				if (h > 0) inpaintingLUT[1][pp] = inpaintingLUT[1][pp - W];
				else inpaintingLUT[1][pp] = -1;

				if (w < W - 1 && h > 0) inpaintingLUT[2][pp] = inpaintingLUT[2][pp + 1 - W];
				else inpaintingLUT[2][pp] = -1;

				if (w > 0) inpaintingLUT[3][pp] = inpaintingLUT[3][pp - 1];
				else inpaintingLUT[3][pp] = -1;
			}

		}//w
	}//h

	//analysis from bottom-right

	for (Int h = H - 1;h >= 0;h--) {
		Int hW = h * W;
		for (Int w = W - 1;w >= 0;w--) {
			Int pp = hW + w;

			inpaintingLUT[4][pp] = pp;
			inpaintingLUT[5][pp] = pp;
			inpaintingLUT[6][pp] = pp;
			inpaintingLUT[7][pp] = pp;

			if (m_yuvOutput[o][0].m_afYUVD[3][pp] == 0) {

				if (w < W - 1 && h < H - 1) inpaintingLUT[4][pp] = inpaintingLUT[4][pp + 1 + W];
				else inpaintingLUT[4][pp] = -1;

				if (h < H - 1) inpaintingLUT[5][pp] = inpaintingLUT[5][pp + W];
				else inpaintingLUT[5][pp] = -1;

				if (w > 0 && h < H - 1) inpaintingLUT[6][pp] = inpaintingLUT[6][pp - 1 + W];
				else inpaintingLUT[6][pp] = -1;

				if (w < W - 1) inpaintingLUT[7][pp] = inpaintingLUT[7][pp + 1];
				else inpaintingLUT[7][pp] = -1;
			}

		}//w
	}//h

	//inpainting

	for (Int h = 0, pp = 0;h < H;h++) {
		for (Int w = 0;w < W;w++, pp++) {

			if (m_yuvOutput[o][0].m_afYUVD[3][pp] != 0) continue;

			Float sumWeight = 0.0;
			Int dist;
			Float weight;
			Float farthestDepth = BIG_FLOAT_VALUE;

			for (UInt d = 0;d < 8;d++) { //farthest depth searching
				if (inpaintingLUT[d][pp] == -1) continue;
				if (farthestDepth > m_yuvOutput[o][0].m_afYUVD[3][inpaintingLUT[d][pp]]) farthestDepth = m_yuvOutput[o][0].m_afYUVD[3][inpaintingLUT[d][pp]];
			}

			for (UInt d = 0;d < 8;d++) { //2-way (horizontal) or 4-way 
				if (inpaintingLUT[d][pp] == -1) continue;

				Int tmph = inpaintingLUT[d][pp] / W;
				Int tmpw = inpaintingLUT[d][pp] % W;

				dist = sqrt((tmph - h)*(tmph - h) + (tmpw - w)*(tmpw - w));

				if (m_yuvOutput[o][0].m_afYUVD[3][inpaintingLUT[d][pp]] - farthestDepth > m_fDepthBlendingThreshold) continue;

				weight = 1.0 / (dist);

				m_yuvOutput[o][0].m_afYUVD[3][pp] += (m_yuvOutput[o][0].m_afYUVD[3][inpaintingLUT[d][pp]] * weight);

				sumWeight += weight;
			}

			if (sumWeight > 0) {

				m_yuvOutput[o][0].m_afYUVD[3][pp] /= sumWeight;

			}

		}//w
	}//h

	for (UInt d = 0;d < 8;d++) delete inpaintingLUT[d];
	delete inpaintingLUT;

	return;
}

void Refiner::inpaintSmallAreas(UInt o) {

	Int M = 3;

	Int H = m_yuvOutput[o]->m_iHeight;
	Int W = m_yuvOutput[o]->m_iWidth;

	for (Int m = 1;m <= M;m++) {
		Int mh = m;
		Int mw = m;
		for (Int h = mh;h < H - mh;h++) {
			Int hW = h * W;
			Int mhW = mh * W;
			for (Int w = mw;w < W - mw;w++) {
				Int pp = hW + w;

				if (m_yuvOutput[o][0].m_afYUVD[3][pp] != 0 && m_yuvOutput[o][0].m_afYUVD[3][pp] < m_yuvOutput[o][0].m_afYUVD[3][pp - mw] - m_fDepthBlendingThreshold && m_yuvOutput[o][0].m_afYUVD[3][pp] < m_yuvOutput[o][0].m_afYUVD[3][pp + mw] - m_fDepthBlendingThreshold) {
					if (abs(m_yuvOutput[o][0].m_afYUVD[3][pp - mw] - m_yuvOutput[o][0].m_afYUVD[3][pp + mw]) > m_fDepthBlendingThreshold) continue;
					m_yuvOutput[o][0].m_afYUVD[3][pp] = (m_yuvOutput[o][0].m_afYUVD[3][pp - mw] + m_yuvOutput[o][0].m_afYUVD[3][pp + mw]) / 2;
				}
				else if (m_yuvOutput[o][0].m_afYUVD[3][pp] != 0 && m_yuvOutput[o][0].m_afYUVD[3][pp] < m_yuvOutput[o][0].m_afYUVD[3][pp - mhW] - m_fDepthBlendingThreshold && m_yuvOutput[o][0].m_afYUVD[3][pp] < m_yuvOutput[o][0].m_afYUVD[3][pp + mhW] - m_fDepthBlendingThreshold) {
					if (abs(m_yuvOutput[o][0].m_afYUVD[3][pp - mhW] - m_yuvOutput[o][0].m_afYUVD[3][pp + mhW]) > m_fDepthBlendingThreshold) continue;
					m_yuvOutput[o][0].m_afYUVD[3][pp] = (m_yuvOutput[o][0].m_afYUVD[3][pp - mhW] + m_yuvOutput[o][0].m_afYUVD[3][pp + mhW]) / 2;
				}

			}
		}
	}

	return;
}

template <typename T>
void Refiner::qSort(T tab[], Int left, Int right) {

	Int i = left;
	Int j = right;
	T x = tab[(left + right) / 2];
	do {
		while (tab[i] < x) i++;
		while (tab[j] > x) j--;
		if (i <= j)	std::swap(tab[i++], tab[j--]);
	} while (i <= j);

	if (left < j) qSort<T>(tab, left, j);
	if (right > i) qSort<T>(tab, i, right);

	return;
}

void Refiner::CE5_ColorRefinement(UInt i, UInt frame) {

	Int iRefCamId = 0;
	Int iNumberOfColorRanges = 3;
	Int iOverlapInPercent = 99;

	if (i == iRefCamId) {
		for (UInt h = 0, pp = 0;h < m_yuvOutput[i]->m_iHeight;h++) {
			for (UInt w = 0;w < m_yuvOutput[i]->m_iWidth;w++, pp++) {
				for (UInt c = 0;c < 3;c++) {
					m_yuvOutput[i]->m_afYUVD[c][pp] = m_yuvIntermediate[i][i]->m_afYUVD[c][pp];
				}
			}
		}

		return;
	}

	Double dOverlap = iOverlapInPercent * 0.005 / iNumberOfColorRanges;
	Double **colorRanges = new Double*[iNumberOfColorRanges];
	for (UInt r = 0;r < iNumberOfColorRanges;r++) {
		colorRanges[r] = new Double[5];
		colorRanges[r][2] = (0.5 + r) / iNumberOfColorRanges;
		colorRanges[r][1] = colorRanges[r][2] - 0.5 / iNumberOfColorRanges;
		colorRanges[r][3] = colorRanges[r][2] + 0.5 / iNumberOfColorRanges;
		colorRanges[r][0] = Max(colorRanges[r][1] - dOverlap, 0);
		colorRanges[r][4] = Min(colorRanges[r][3] + dOverlap, 1);
	}

	Double **offset = new Double*[3]; //RGB
	for (UInt c = 0;c < 3;c++) {
		offset[c] = new Double[iNumberOfColorRanges];
		for (UInt r = 0;r < iNumberOfColorRanges;r++) {
			offset[c][r] = 0;
		}
	}

	Int **counter = new Int*[3]; //RGB
	for (UInt c = 0;c < 3;c++) {
		counter[c] = new Int[iNumberOfColorRanges];
		for (UInt r = 0;r < iNumberOfColorRanges;r++) {
			counter[c][r] = 0;
		}
	}

	for (UInt h = 0, pp = 0;h < m_yuvOutput[i]->m_iHeight;h++) {
		for (UInt w = 0;w < m_yuvOutput[i]->m_iWidth;w++, pp++) {

			if (m_yuvIntermediate[iRefCamId][iRefCamId]->m_afYUVD[3][pp] == 0) continue;

			if (m_yuvIntermediate[iRefCamId][i]->m_afYUVD[3][pp]) {

				for (UInt c = 0;c < 3;c++) {
					for (UInt r = 0;r < iNumberOfColorRanges;r++) {
						if (m_yuvIntermediate[iRefCamId][iRefCamId]->m_afYUVD[c][pp] >= colorRanges[r][0] && m_yuvIntermediate[iRefCamId][iRefCamId]->m_afYUVD[c][pp] <= colorRanges[r][4]) {
							offset[c][r] += (m_yuvIntermediate[iRefCamId][i]->m_afYUVD[c][pp] - m_yuvIntermediate[iRefCamId][iRefCamId]->m_afYUVD[c][pp]);
							counter[c][r]++;
						}
					}
				}
			}

		}
	}

	for (UInt c = 0;c < 3;c++) {
		for (UInt r = 0;r < iNumberOfColorRanges;r++) {
			if (counter[c][r]) offset[c][r] /= counter[c][r];
		}
	}

	for (UInt c = 0, ii = 0;c < 3;c++) {
		for (UInt r = 0;r < iNumberOfColorRanges;r++, ii++) {
			if (!frame) m_IIR[i].push_back(offset[c][r]);
			else {
				m_IIR[i][ii] = m_IIR[i][ii] * m_dWeightIIR + offset[c][r] * (1 - m_dWeightIIR);
				offset[c][r] = m_IIR[i][ii];
			}
		}
	}


	for (UInt h = 0, pp = 0;h < m_yuvOutput[i]->m_iHeight;h++) {
		for (UInt w = 0;w < m_yuvOutput[i]->m_iWidth;w++, pp++) {

			for (UInt c = 0;c < 3;c++) {

				Int r0 = -1, r1 = -1;
				for (UInt r = 0;r < iNumberOfColorRanges;r++) {
					if (m_yuvIntermediate[i][i]->m_afYUVD[c][pp] >= colorRanges[r][0] && m_yuvIntermediate[i][i]->m_afYUVD[c][pp] <= colorRanges[r][4]) {
						if (r0 < 0) r0 = r;
						else r1 = r;
					}
					if (r1 > 0) continue;
				}

				if (r1 < 0) m_yuvOutput[i]->m_afYUVD[c][pp] = m_yuvIntermediate[i][i]->m_afYUVD[c][pp] - offset[c][r0];
				else {
					Double d0 = abs(m_yuvIntermediate[i][i]->m_afYUVD[c][pp] - colorRanges[r1][2]);
					Double d1 = abs(m_yuvIntermediate[i][i]->m_afYUVD[c][pp] - colorRanges[r0][2]);
					Double dd = d0 + d1;
					d0 /= dd;
					d1 /= dd;
					m_yuvOutput[i]->m_afYUVD[c][pp] = m_yuvIntermediate[i][i]->m_afYUVD[c][pp];
					m_yuvOutput[i]->m_afYUVD[c][pp] -= (offset[c][r0] * d0);
					m_yuvOutput[i]->m_afYUVD[c][pp] -= (offset[c][r1] * d1);
				}

			}

		}
	}

	for (UInt r = 0;r < iNumberOfColorRanges;r++) delete colorRanges[r];
	delete colorRanges;

	delete offset[0];
	delete offset[1];
	delete offset[2];
	delete offset;

	delete counter[0];
	delete counter[1];
	delete counter[2];
	delete counter;

	return;
}

void Refiner::temphance(UInt i) {

	Double maxDiffY = 0.1;
	Double maxDiffU = 0.05;	
	Double maxDiffV = 0.05;
	
	Int BW = m_yuvInput[0]->m_iWidth / BACKGROUND_WH_STEP;
	Int BH = m_yuvInput[0]->m_iHeight / BACKGROUND_WH_STEP;

	Double diff[3] = { 0 };
	Int cnt = 0;

	for (Int h = 0, ppb = 0;h < BH;h++) {
		for (Int w = 0;w < BW;w++, ppb++) {
			Int pp = (h * BACKGROUND_WH_STEP) * m_yuvInput[i]->m_iWidth + (w * BACKGROUND_WH_STEP);
			Double diffY = (m_yuvInput[i]->m_afYUVD[0][pp] - m_yuvBackground[i][0][ppb]);
			Double diffU = (m_yuvInput[i]->m_afYUVD[1][pp] - m_yuvBackground[i][1][ppb]);
			Double diffV = (m_yuvInput[i]->m_afYUVD[2][pp] - m_yuvBackground[i][2][ppb]);

			if(abs(diffY) < maxDiffY && abs(diffU) < maxDiffU && abs(diffV) < maxDiffV){
				diff[0] += diffY;
				diff[1] += diffU;
				diff[2] += diffV;
				cnt++;
			}
		}
	}

	if (cnt) for (Int c = 0;c < 3;c++) diff[c] /= cnt;

	for (Int h = 0, pp = 0;h < m_yuvInput[i]->m_iHeight;h++) {
		for (Int w = 0;w < m_yuvInput[i]->m_iWidth;w++, pp++) {
			for (Int c = 0;c < 3;c++) {
				m_yuvInput[i]->m_afYUVD[c][pp] -= diff[c];
			}
		}
	}

	return;
}

void Refiner::refine(UInt frame) {

#if ESTIMATE_TIME
	clock_t tStart = clock();
#endif

	for (UInt i = 0;i < m_uiNumberOfInputViews;i++) {
		temphance(i);
	}

	std::cout << "\nProjecting to view:\t";

	for (UInt o = 0;o < m_uiNumberOfOutputViews;o++) {
		std::cout << o << " ";

		UInt outFormat = m_yuvOutput[o]->m_uiFormat;

		m_yuvOutput[o]->m_sCurrentCameraName = m_yuvOutput[o]->m_asCameraNames[frame % m_yuvOutput[o]->m_asCameraNames.size()];
		m_yuvOutput[o]->m_currentCamParams = m_yuvOutput[o]->m_aCamParams[frame % m_yuvOutput[o]->m_aCamParams.size()];

		for (UInt i = 0;i < m_uiNumberOfInputViews;i++) {
			UInt inFormat = m_yuvInput[i]->m_uiFormat;

			if (i == o) {
				for (UInt pp = 0;pp < m_yuvInput[i]->m_uiDepthLumaFrameSizeInPixels;pp++) {
					for (UInt c = 0;c < 4;c++) m_yuvIntermediate[o][i]->m_afYUVD[c][pp] = m_yuvInput[i]->m_afYUVD[c][pp];
				}
			}
			else {
				if (m_uiSynthesisMethod == 1) {
					projectDepth(i, o, inFormat, outFormat);
					//filterDepth(i, o);
					projectTexture(i, o, inFormat, outFormat);
				}
			}
		}
	}

	std::cout << "\nColor refinement:\t";
	for (UInt i = 0;i < m_uiNumberOfInputViews;i++) {
		std::cout << i << " ";
		CE5_ColorRefinement(i, frame);
	}

	cvtProcessingArraysToOutput();

#if ESTIMATE_TIME
	clock_t tEnd = clock();
	std::cout << "\nProcessing time:  \t" << (tEnd - tStart) << " ms\n";
#endif

	return;
}

void Refiner::initBackgroundVector(UInt numberOfFrames) {

	Int BW = m_yuvInput[0]->m_iWidth / BACKGROUND_WH_STEP;
	Int BH = m_yuvInput[0]->m_iHeight / BACKGROUND_WH_STEP;

	m_iBackgroundVectorSize = numberOfFrames / BACKGROUND_T_STEP;

	m_yuvBackgroundVector = new Double ***[m_uiNumberOfInputViews];
	for (Int i = 0;i < m_uiNumberOfInputViews;i++) {
		m_yuvBackgroundVector[i] = new Double **[3];
		for (Int c = 0;c < 3;c++) {
			m_yuvBackgroundVector[i][c] = new Double *[BW * BH];
			for (Int pp = 0;pp < BW*BH;pp++) {
				m_yuvBackgroundVector[i][c][pp] = new Double[m_iBackgroundVectorSize];
			}
		}
	}

	return;
}

void Refiner::deinitBackgroundVector() {

	Int BW = m_yuvInput[0]->m_iWidth / BACKGROUND_WH_STEP;
	Int BH = m_yuvInput[0]->m_iHeight / BACKGROUND_WH_STEP;

	for (Int i = 0;i < m_uiNumberOfInputViews;i++) {
		for (Int c = 0;c < 3;c++) {
			for (Int pp = 0;pp < BW*BH;pp++) {
				delete m_yuvBackgroundVector[i][c][pp];
			}
			delete m_yuvBackgroundVector[i][c];
		}
		delete m_yuvBackgroundVector[i];
	}
	delete m_yuvBackgroundVector;

	return;
}

void Refiner::fillBackgroundVector(UInt frame) {

	Int BW = m_yuvInput[0]->m_iWidth / BACKGROUND_WH_STEP;
	Int BH = m_yuvInput[0]->m_iHeight / BACKGROUND_WH_STEP;

	for (Int i = 0;i < m_uiNumberOfInputViews;i++) {
		for (Int c = 0;c < 3;c++) {
			for (Int h = 0, ppb = 0;h < BH;h++) {
				for (Int w = 0;w < BW;w++, ppb++) {
					Int pp = (h * BACKGROUND_WH_STEP) * m_yuvInput[i]->m_iWidth + (w * BACKGROUND_WH_STEP);
					m_yuvBackgroundVector[i][c][ppb][frame] = m_yuvInput[i]->m_afYUVD[c][pp];
				}
			}
		}
	}

	return;
}

void Refiner::calcBackground() {

	Int BW = m_yuvInput[0]->m_iWidth / BACKGROUND_WH_STEP;
	Int BH = m_yuvInput[0]->m_iHeight / BACKGROUND_WH_STEP;

	for (Int i = 0;i < m_uiNumberOfInputViews;i++) {
		for (Int c = 0;c < 3;c++) {
			for (Int pp = 0;pp < BW*BH;pp++) {
				qSort<Double>(m_yuvBackgroundVector[i][c][pp], 0, m_iBackgroundVectorSize - 1);
				m_yuvBackground[i][c][pp] = m_yuvBackgroundVector[i][c][pp][m_iBackgroundVectorSize / 2];
			}
		}
	}

	return;
}
