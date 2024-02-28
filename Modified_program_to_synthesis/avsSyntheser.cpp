#include "avsSyntheser.h"

Syntheser::~Syntheser() {

	return;
}

Syntheser::Syntheser(cArray<YUV*> yuvIn, cArray<YUV*> yuvOut, Config* cfg) {

	m_uiNumberOfInputViews = yuvIn.size();
	m_uiNumberOfOutputViews = yuvOut.size();

	m_yuvInput = yuvIn;
	m_yuvOutput = yuvOut;

	for (UInt o = 0; o < m_uiNumberOfOutputViews; o++) {
		cArray<YUV*> inter2;
		for (UInt i = 0; i < m_uiNumberOfInputViews; i++) {
			YUV* inter = new YUV(m_yuvOutput[o]->m_uiWidth, m_yuvOutput[o]->m_uiHeight);
			inter2.push_back(inter);
		}
		m_yuvIntermediate.push_back(inter2);
	}
	m_uiPrioritization = cfg->m_uiPrioritization;
	m_uiDepthBasedPrioritization = cfg->m_uiDepthBasedPrioritization;

	Float fInvMaxDepth = 1.0 / ((1 << cfg->m_uiDepthBitsPerSample) - 1);
	m_fDepthBlendingThreshold = cfg->m_fDepthBlendingThreshold * fInvMaxDepth * (1.0 / cfg->m_dZNear - 1.0 / cfg->m_dZFar);

	m_uiRemoveGhostEdges = cfg->m_uiRemoveGhostEdges;

	m_uiMinPriority = m_yuvInput[0]->m_uiPriority;
	m_uiMaxPriority = m_yuvInput[0]->m_uiPriority;

	for (UInt i = 0; i < m_uiNumberOfInputViews; i++) {
		if (m_uiMinPriority > m_yuvInput[i]->m_uiPriority) m_uiMinPriority = m_yuvInput[i]->m_uiPriority;
		if (m_uiMaxPriority < m_yuvInput[i]->m_uiPriority) m_uiMaxPriority = m_yuvInput[i]->m_uiPriority;
	}

	return;
}

void Syntheser::cvtInputArraysToProcessing() {

	for (UInt i = 0; i < m_uiNumberOfInputViews; i++) {
		m_yuvInput[i]->cvtInputArraysToProcessing();
	}

	return;
}

void Syntheser::cvtProcessingArraysToOutput() {

	for (UInt o = 0; o < m_uiNumberOfOutputViews; o++) {
		m_yuvOutput[o]->cvtProcessingArraysToOutput();
	}

	return;
}

void Syntheser::clearProcessingArrays() {

	for (UInt o = 0; o < m_uiNumberOfOutputViews; o++) {
		for (UInt c = 0; c < 4; c++) memset(m_yuvOutput[o]->m_afYUVD[c], 0, m_yuvOutput[o]->m_uiViewLumaFrameSizeInPixels * 4);
		for (UInt i = 0; i < m_uiNumberOfInputViews; i++) {
			for (UInt c = 0; c < 4; c++) memset(m_yuvIntermediate[o][i]->m_afYUVD[c], BIG_FLOAT_VALUE, m_yuvIntermediate[o][i]->m_uiViewLumaFrameSizeInPixels * 4);
		}
	}
	//std::cout << m_yuvIntermediate.size() << " " << m_yuvIntermediate[0].size() << std::endl;
	return;
}

void Syntheser::removeGhostEdges() {

	Int edgeSize = m_uiRemoveGhostEdges;
	Int numberOfCheckedNeighbors = edgeSize * 2;

	Int margin = edgeSize + numberOfCheckedNeighbors;

	Float depthThresh = m_fDepthBlendingThreshold;

	for (UInt i = 0; i < m_uiNumberOfInputViews; i++) {

		Int H = m_yuvInput[i]->m_uiHeight;
		Int W = m_yuvInput[i]->m_uiWidth;

		Float diffY, diffU, diffV;
		Float anY, anU, anV;

		Float* tmpArray = new Float[m_yuvInput[i]->m_uiViewLumaFrameSizeInPixels];
		for (UInt pp = 0; pp < m_yuvInput[i]->m_uiViewLumaFrameSizeInPixels; pp++) tmpArray[pp] = 0;

		for (Int h = margin; h < H - margin; h++) {
			Int hW = h * W;
			for (Int w = margin; w < W - margin; w++) {
				Int pp = hW + w;

				for (Int e = 1; e <= edgeSize; e++) {

					if (m_yuvInput[i]->m_afYUVD[3][pp] > m_yuvInput[i]->m_afYUVD[3][pp - e] + depthThresh) { //left is closer

						diffY = 0, diffU = 0, diffV = 0;
						for (Int n = 1; n <= numberOfCheckedNeighbors; n++) {
							diffY += abs(m_yuvInput[i]->m_afYUVD[0][pp + n] - m_yuvInput[i]->m_afYUVD[0][pp + n + 1]);
							diffU += abs(m_yuvInput[i]->m_afYUVD[1][pp + n] - m_yuvInput[i]->m_afYUVD[1][pp + n + 1]);
							diffV += abs(m_yuvInput[i]->m_afYUVD[2][pp + n] - m_yuvInput[i]->m_afYUVD[2][pp + n + 1]);
						}
						anY = (abs(m_yuvInput[i]->m_afYUVD[0][pp] - m_yuvInput[i]->m_afYUVD[0][pp + 1])) * numberOfCheckedNeighbors * 5;
						anU = (abs(m_yuvInput[i]->m_afYUVD[1][pp] - m_yuvInput[i]->m_afYUVD[1][pp + 1])) * numberOfCheckedNeighbors * 5;
						anV = (abs(m_yuvInput[i]->m_afYUVD[2][pp] - m_yuvInput[i]->m_afYUVD[2][pp + 1])) * numberOfCheckedNeighbors * 5;

						if (anY > diffY || anU > diffU || anV > diffV)
							tmpArray[pp] = -1;
						//tmpArray[pp - e] = -1;
					}

					if (m_yuvInput[i]->m_afYUVD[3][pp] > m_yuvInput[i]->m_afYUVD[3][pp + e] + depthThresh) { //right is closer

						diffY = 0, diffU = 0, diffV = 0;
						for (Int n = 1; n <= numberOfCheckedNeighbors; n++) {
							diffY += abs(m_yuvInput[i]->m_afYUVD[0][pp - n] - m_yuvInput[i]->m_afYUVD[0][pp - n - 1]);
							diffU += abs(m_yuvInput[i]->m_afYUVD[1][pp - n] - m_yuvInput[i]->m_afYUVD[1][pp - n - 1]);
							diffV += abs(m_yuvInput[i]->m_afYUVD[2][pp - n] - m_yuvInput[i]->m_afYUVD[2][pp - n - 1]);
						}
						anY = (abs(m_yuvInput[i]->m_afYUVD[0][pp] - m_yuvInput[i]->m_afYUVD[0][pp - 1])) * numberOfCheckedNeighbors * 5;
						anU = (abs(m_yuvInput[i]->m_afYUVD[1][pp] - m_yuvInput[i]->m_afYUVD[1][pp - 1])) * numberOfCheckedNeighbors * 5;
						anV = (abs(m_yuvInput[i]->m_afYUVD[2][pp] - m_yuvInput[i]->m_afYUVD[2][pp - 1])) * numberOfCheckedNeighbors * 5;

						if (anY > diffY || anU > diffU || anV > diffV)
							tmpArray[pp] = -1;
						//tmpArray[pp + e] = -1;
					}

					if (m_yuvInput[i]->m_afYUVD[3][pp] > m_yuvInput[i]->m_afYUVD[3][pp - e * W] + depthThresh) { //top is closer

						diffY = 0, diffU = 0, diffV = 0;
						for (Int n = 1; n <= numberOfCheckedNeighbors; n++) {
							Int nW = n * W;
							diffY += abs(m_yuvInput[i]->m_afYUVD[0][pp + nW] - m_yuvInput[i]->m_afYUVD[0][pp + nW + W]);
							diffU += abs(m_yuvInput[i]->m_afYUVD[1][pp + nW] - m_yuvInput[i]->m_afYUVD[1][pp + nW + W]);
							diffV += abs(m_yuvInput[i]->m_afYUVD[2][pp + nW] - m_yuvInput[i]->m_afYUVD[2][pp + nW + W]);
						}
						anY = (abs(m_yuvInput[i]->m_afYUVD[0][pp] - m_yuvInput[i]->m_afYUVD[0][pp + W])) * numberOfCheckedNeighbors;
						anU = (abs(m_yuvInput[i]->m_afYUVD[1][pp] - m_yuvInput[i]->m_afYUVD[1][pp + W])) * numberOfCheckedNeighbors;
						anV = (abs(m_yuvInput[i]->m_afYUVD[2][pp] - m_yuvInput[i]->m_afYUVD[2][pp + W])) * numberOfCheckedNeighbors;

						if (anY > diffY || anU > diffU || anV > diffV)
							tmpArray[pp] = -1;
						//tmpArray[pp - e * W] = -1;
					}

					if (m_yuvInput[i]->m_afYUVD[3][pp] > m_yuvInput[i]->m_afYUVD[3][pp + e * W] + depthThresh) { //bottom is closer

						diffY = 0, diffU = 0, diffV = 0;
						for (Int n = 1; n <= numberOfCheckedNeighbors; n++) {
							Int nW = n * W;
							diffY += abs(m_yuvInput[i]->m_afYUVD[0][pp - nW] - m_yuvInput[i]->m_afYUVD[0][pp - nW - W]);
							diffU += abs(m_yuvInput[i]->m_afYUVD[1][pp - nW] - m_yuvInput[i]->m_afYUVD[1][pp - nW - W]);
							diffV += abs(m_yuvInput[i]->m_afYUVD[2][pp - nW] - m_yuvInput[i]->m_afYUVD[2][pp - nW - W]);
						}
						anY = (abs(m_yuvInput[i]->m_afYUVD[0][pp] - m_yuvInput[i]->m_afYUVD[0][pp - W])) * numberOfCheckedNeighbors;
						anU = (abs(m_yuvInput[i]->m_afYUVD[1][pp] - m_yuvInput[i]->m_afYUVD[1][pp - W])) * numberOfCheckedNeighbors;
						anV = (abs(m_yuvInput[i]->m_afYUVD[2][pp] - m_yuvInput[i]->m_afYUVD[2][pp - W])) * numberOfCheckedNeighbors;

						if (anY > diffY || anU > diffU || anV > diffV)
							tmpArray[pp] = -1;
						//tmpArray[pp + e * W] = -1;
					}

				}//e
			}//w
		}//h

		for (UInt pp = 0; pp < m_yuvInput[i]->m_uiViewLumaFrameSizeInPixels; pp++) {
			if (tmpArray[pp] < 0) m_yuvInput[i]->m_afYUVD[3][pp] = 0;
		}

		delete tmpArray;
	}//i

	return;
}

void Syntheser::calcImPosPersp(Double& wPos, Double& hPos, Double& invTargetZ, Matrix_3x1* XYZ, Matrix_4x4* mat) {

	Matrix_3x1 tmpMat;
	matrixMultiply_4x4_3x1(*mat, *XYZ, tmpMat);

	if (tmpMat.at(2) <= 0) return;

	invTargetZ = 1 / tmpMat.at(2);

	wPos = tmpMat.at(0) * invTargetZ;
	hPos = tmpMat.at(1) * invTargetZ;

	return;
}

void Syntheser::calcImPosOmni(UInt& W, UInt& H, Double& wPos, Double& hPos, Double& invTargetZ, Matrix_3x1* XYZ, Double& OAovL, Double& OAovW, Double& OAovB, Double& OAovH) {

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

void Syntheser::calcXYZPersp(UInt& w, UInt& h, Double& invZ, Matrix_3x1* XYZ, Matrix_4x4* mat) {

	Matrix_3x1 whz;
	Double z = 1.0 / invZ;
	whz.at(0) = w * z;
	whz.at(1) = h * z;
	whz.at(2) = 1 * z;

	matrixMultiply_4x4_3x1(*mat, whz, *XYZ);

	return;
}

template <UInt inFormat, UInt outFormat> void Syntheser::projectTexture(UInt i, UInt o) {

	UInt IW = m_yuvInput[i]->m_uiWidth;
	UInt IH = m_yuvInput[i]->m_uiHeight;

	UInt OW = m_yuvIntermediate[o][i][0].m_uiWidth;
	UInt OH = m_yuvIntermediate[o][i][0].m_uiHeight;

	Double IAovL = m_yuvInput[i]->m_dAovL;
	Double IAovW = m_yuvInput[i]->m_dAovW;
	Double IAovB = m_yuvInput[i]->m_dAovB;
	Double IAovH = m_yuvInput[i]->m_dAovH;

	Double OAovL = m_yuvOutput[o]->m_dAovL;
	Double OAovW = m_yuvOutput[o]->m_dAovW;
	Double OAovB = m_yuvOutput[o]->m_dAovB;
	Double OAovH = m_yuvOutput[o]->m_dAovH;

	Double** LUT_Theta = new Double * [2];
	Double** LUT_Phi = new Double * [2];

	for (UInt i = 0; i < 2; i++) LUT_Theta[i] = new Double[OH];
	for (UInt i = 0; i < 2; i++) LUT_Phi[i] = new Double[OW];

	for (UInt h = 0; h < OH; h++) {
		Double theta = Double(h) * OAovH / (OH - 1) + OAovB;
		LUT_Theta[0][h] = cos(theta);
		LUT_Theta[1][h] = sin(theta);
	}
	for (UInt w = 0; w < OW; w++) {
		Double phi = Double(w) * OAovW / (OW - 1) + OAovL;
		LUT_Phi[0][w] = cos(phi);
		LUT_Phi[1][w] = sin(phi);
	}

	Matrix_3x1* XYZ = new Matrix_3x1;
	Matrix_3x1* XYZ2 = new Matrix_3x1;

	Double wPos, hPos, dInvZ, dInvTargetZ;

	Matrix_4x4 R1;
	Matrix_4x4 R2;
	for (UInt h = 0; h < 3; h++) {
		for (UInt w = 0; w < 3; w++) R2.at(h, w) = m_yuvOutput[o]->m_currentCamParams->m_mCvtdExtMat.at(h, w);
	}
	matrixInversion_4x4(R2, R1);
	for (UInt h = 0; h < 3; h++) {
		for (UInt w = 0; w < 3; w++) R2.at(h, w) = m_yuvInput[i]->m_currentCamParams->m_mCvtdExtMat.at(h, w);
	}

	for (UInt h = 0; h < OH; h++) {
		UInt hW = h * OW;
		for (UInt w = 0; w < OW; w++) {
			UInt pp = hW + w;

			dInvZ = m_yuvIntermediate[o][i]->m_afYUVD[3][pp];
			if (dInvZ == 0) continue;

			if constexpr (outFormat == 0)
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

			if constexpr (inFormat == 0)
				calcImPosPersp(wPos, hPos, dInvTargetZ, XYZ, &(m_yuvInput[i]->m_currentCamParams->m_mProjMat));
			else {
				XYZ->at(0) -= m_yuvInput[i]->m_currentCamParams->m_mExtMat.at(0, 3);
				XYZ->at(1) -= m_yuvInput[i]->m_currentCamParams->m_mExtMat.at(1, 3);
				XYZ->at(2) -= m_yuvInput[i]->m_currentCamParams->m_mExtMat.at(2, 3);
				matrixMultiply_4x4_3x1(R2, *XYZ, *XYZ2);
				calcImPosOmni(IW, IH, wPos, hPos, dInvTargetZ, XYZ2, IAovL, IAovW, IAovB, IAovH);
			}

			Int tmppp = Int(hPos) * IW + Int(wPos);

			if (Int(wPos) < 0 || Int(wPos + 1) >= IW || Int(hPos) < 0 || Int(hPos + 1) >= IH) copyColorFromInput(i, o, pp, wPos, hPos, false);
			else if (m_yuvInput[i]->m_afYUVD[3][tmppp] * m_yuvInput[i]->m_afYUVD[3][tmppp + 1] * m_yuvInput[i]->m_afYUVD[3][tmppp + IW] * m_yuvInput[i]->m_afYUVD[3][tmppp + IW + 1] == 0) copyColorFromInput(i, o, pp, wPos, hPos, false);
			else copyColorFromInput(i, o, pp, wPos, hPos, false);

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

template <UInt inFormat, UInt outFormat> void Syntheser::projectDepth(UInt i, UInt o) {

	UInt IW = m_yuvInput[i]->m_uiWidth;
	UInt IH = m_yuvInput[i]->m_uiHeight;

	UInt OW = m_yuvIntermediate[o][i][0].m_uiWidth;
	UInt OH = m_yuvIntermediate[o][i][0].m_uiHeight;

	Double IAovL = m_yuvInput[i]->m_dAovL;
	Double IAovW = m_yuvInput[i]->m_dAovW;
	Double IAovB = m_yuvInput[i]->m_dAovB;
	Double IAovH = m_yuvInput[i]->m_dAovH;

	Double OAovL = m_yuvOutput[o]->m_dAovL;
	Double OAovW = m_yuvOutput[o]->m_dAovW;
	Double OAovB = m_yuvOutput[o]->m_dAovB;
	Double OAovH = m_yuvOutput[o]->m_dAovH;

	Double** LUT_Theta = new Double * [2];
	Double** LUT_Phi = new Double * [2];

	for (UInt i = 0; i < 2; i++) LUT_Theta[i] = new Double[IH];
	for (UInt i = 0; i < 2; i++) LUT_Phi[i] = new Double[IW];

	for (UInt h = 0; h < IH; h++) {
		Double theta = Double(h) * IAovH / (IH - 1) + IAovB;
		LUT_Theta[0][h] = cos(theta);
		LUT_Theta[1][h] = sin(theta);
	}
	for (UInt w = 0; w < IW; w++) {
		Double phi = Double(w) * IAovW / (IW - 1) + IAovL;
		LUT_Phi[0][w] = cos(phi);
		LUT_Phi[1][w] = sin(phi);
	}

	Matrix_3x1* XYZ = new Matrix_3x1;
	Matrix_3x1* XYZ2 = new Matrix_3x1;

	Matrix_4x4 R1;
	Matrix_4x4 R2;
	for (UInt h = 0; h < 3; h++) {
		for (UInt w = 0; w < 3; w++) R2.at(h, w) = m_yuvInput[i]->m_currentCamParams->m_mCvtdExtMat.at(h, w);
	}
	matrixInversion_4x4(R2, R1);
	for (UInt h = 0; h < 3; h++) {
		for (UInt w = 0; w < 3; w++) R2.at(h, w) = m_yuvOutput[o]->m_currentCamParams->m_mCvtdExtMat.at(h, w);
	}

	Double wPos, hPos, dInvZ, dInvTargetZ;

	for (UInt h = 0; h < IH; h++) {
		UInt hW = h * IW;
		for (UInt w = 0; w < IW; w++) {
			UInt pp = hW + w;

			dInvZ = m_yuvInput[i]->m_afYUVD[3][pp];
			if (dInvZ == 0) continue;

			if constexpr (inFormat == 0)
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

			if constexpr (outFormat == 0)
				calcImPosPersp(wPos, hPos, dInvTargetZ, XYZ, &(m_yuvOutput[o]->m_currentCamParams->m_mProjMat));
			else {
				XYZ->at(0) -= m_yuvOutput[o]->m_currentCamParams->m_mExtMat.at(0, 3);
				XYZ->at(1) -= m_yuvOutput[o]->m_currentCamParams->m_mExtMat.at(1, 3);
				XYZ->at(2) -= m_yuvOutput[o]->m_currentCamParams->m_mExtMat.at(2, 3);

				matrixMultiply_4x4_3x1(R2, *XYZ, *XYZ2);

				calcImPosOmni(OW, OH, wPos, hPos, dInvTargetZ, XYZ2, OAovL, OAovW, OAovB, OAovH);
			}

			if (wPos >= 0 && wPos <= OW - 1 && hPos >= 0 && hPos <= OH - 1) {

				UInt ppt = Int(hPos + 0.5) * OW + Int(wPos + 0.5);

				if (m_yuvIntermediate[o][i]->m_afYUVD[3][ppt] > dInvTargetZ) {
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

void Syntheser::filterDepth(UInt i, UInt o) {
	Int holeSize = 2;

	Float tmpVal[4];

	Int H = m_yuvIntermediate[o][i][0].m_uiHeight;
	Int W = m_yuvIntermediate[o][i][0].m_uiWidth;

	for (UInt m = 1; m <= holeSize; m++) {
		for (Int h = m; h < H - m; h++) {
			Int hW = h * W;
			Int mW = m * W;
			for (Int w = m; w < W - m; w++) {
				Int pp = hW + w;

				for (Int v = 0; v < 4; v++) tmpVal[v] = 0;

				if (m_yuvIntermediate[o][i]->m_afYUVD[3][pp - m] != 0 && m_yuvIntermediate[o][i]->m_afYUVD[3][pp + m] != 0) {
					if (m_yuvIntermediate[o][i]->m_afYUVD[3][pp] == 0 || m_yuvIntermediate[o][i]->m_afYUVD[3][pp] > Max(m_yuvIntermediate[o][i]->m_afYUVD[3][pp - m], m_yuvIntermediate[o][i]->m_afYUVD[3][pp + m])) {
						tmpVal[0] = Max(m_yuvIntermediate[o][i]->m_afYUVD[3][pp - m], m_yuvIntermediate[o][i]->m_afYUVD[3][pp + m]);
					}
				}
				if (m_yuvIntermediate[o][i]->m_afYUVD[3][pp - mW] != 0 && m_yuvIntermediate[o][i]->m_afYUVD[3][pp + mW] != 0) {
					if (m_yuvIntermediate[o][i]->m_afYUVD[3][pp] == 0 || m_yuvIntermediate[o][i]->m_afYUVD[3][pp] > Max(m_yuvIntermediate[o][i]->m_afYUVD[3][pp - mW], m_yuvIntermediate[o][i]->m_afYUVD[3][pp + mW])) {
						tmpVal[1] = Max(m_yuvIntermediate[o][i]->m_afYUVD[3][pp - mW], m_yuvIntermediate[o][i]->m_afYUVD[3][pp + mW]);
					}
				}
				if (m_yuvIntermediate[o][i]->m_afYUVD[3][pp - m - mW] != 0 && m_yuvIntermediate[o][i]->m_afYUVD[3][pp + m + mW] != 0) {
					if (m_yuvIntermediate[o][i]->m_afYUVD[3][pp] == 0 || m_yuvIntermediate[o][i]->m_afYUVD[3][pp] > Max(m_yuvIntermediate[o][i]->m_afYUVD[3][pp - m - mW], m_yuvIntermediate[o][i]->m_afYUVD[3][pp + m + mW])) {
						tmpVal[2] = Max(m_yuvIntermediate[o][i]->m_afYUVD[3][pp - m - mW], m_yuvIntermediate[o][i]->m_afYUVD[3][pp + m + mW]);
					}
				}
				if (m_yuvIntermediate[o][i]->m_afYUVD[3][pp - m + mW] != 0 && m_yuvIntermediate[o][i]->m_afYUVD[3][pp + m - mW] != 0) {
					if (m_yuvIntermediate[o][i]->m_afYUVD[3][pp] == 0 || m_yuvIntermediate[o][i]->m_afYUVD[3][pp] > Max(m_yuvIntermediate[o][i]->m_afYUVD[3][pp - m + mW], m_yuvIntermediate[o][i]->m_afYUVD[3][pp + m - mW])) {
						tmpVal[3] = Max(m_yuvIntermediate[o][i]->m_afYUVD[3][pp - m + mW], m_yuvIntermediate[o][i]->m_afYUVD[3][pp + m - mW]);
					}
				}

				Float maxVal = 0;
				Int tmpCnt = 0;
				for (Int v = 0; v < 4; v++) {
					if (!tmpVal[v]) continue;
					if (maxVal < tmpVal[v]) maxVal = tmpVal[v];
					tmpCnt++;
				}
				if (tmpCnt) m_yuvIntermediate[o][i]->m_afYUVD[0][pp] = maxVal;
				else m_yuvIntermediate[o][i]->m_afYUVD[0][pp] = 0;

			}//w
		}//h

		for (Int h = m; h < H - m; h++) {
			Int hW = h * W;
			for (Int w = m; w < W - m; w++) {
				Int pp = hW + w;

				if (m_yuvIntermediate[o][i]->m_afYUVD[0][pp] == 0) continue;
				m_yuvIntermediate[o][i]->m_afYUVD[3][pp] = m_yuvIntermediate[o][i]->m_afYUVD[0][pp];
				m_yuvIntermediate[o][i]->m_afYUVD[0][pp] = 0;

			}//w
		}//h
	}//m

	return;
}

void Syntheser::copyColorFromInput(UInt& i, UInt& o, UInt& pp, Double& wPos, Double& hPos, Bool interpolate) {

#if !INTERPOLATE_COLOR_USING_4_PIXELS
	interpolate = false;
#endif

	if (wPos >= 0 && wPos < m_yuvInput[i]->m_uiWidth - 1 && hPos >= 0 && hPos < m_yuvInput[i]->m_uiHeight - 1) {
		if (!interpolate) {

			UInt ppt = Int(hPos + .5) * m_yuvInput[i]->m_uiWidth + Int(wPos + .5);
			if (m_yuvInput[i]->m_afYUVD[3][ppt] == 0) {
				m_yuvIntermediate[o][i]->m_afYUVD[3][pp] = 0;
				return;
			}

			m_yuvIntermediate[o][i]->m_afYUVD[0][pp] = m_yuvInput[i]->m_afYUVD[0][ppt];
			m_yuvIntermediate[o][i]->m_afYUVD[1][pp] = m_yuvInput[i]->m_afYUVD[1][ppt];
			m_yuvIntermediate[o][i]->m_afYUVD[2][pp] = m_yuvInput[i]->m_afYUVD[2][ppt];

		}
		else {
			Int iTargetH = Int(hPos);
			Int iTargetW = Int(wPos);

			Double distL = wPos - iTargetW;
			Double distR = iTargetW + 1 - wPos;
			Double distT = hPos - iTargetH;
			Double distB = iTargetH + 1 - hPos;

			Double distLT = 1.0 / sqrt(distL * distL + distT * distT);
			Double distRT = 1.0 / sqrt(distR * distR + distT * distT);
			Double distLB = 1.0 / sqrt(distL * distL + distB * distB);
			Double distRB = 1.0 / sqrt(distR * distR + distB * distB);


			Double distSum = distLT + distRT + distLB + distRB;

			for (UInt c = 0; c < 3; c++) {
				Double valLT = distLT * m_yuvInput[i]->m_afYUVD[c][iTargetH * m_yuvInput[i]->m_uiWidth + iTargetW];
				Double valRT = distRT * m_yuvInput[i]->m_afYUVD[c][iTargetH * m_yuvInput[i]->m_uiWidth + iTargetW + 1];
				Double valLB = distLB * m_yuvInput[i]->m_afYUVD[c][(iTargetH + 1) * m_yuvInput[i]->m_uiWidth + iTargetW];
				Double valRB = distRB * m_yuvInput[i]->m_afYUVD[c][(iTargetH + 1) * m_yuvInput[i]->m_uiWidth + iTargetW + 1];

				m_yuvIntermediate[o][i]->m_afYUVD[c][pp] = (valLT + valRT + valLB + valRB) / distSum;
			}

		}
	}
	else {
		m_yuvIntermediate[o][i]->m_afYUVD[3][pp] = 0; //clear depth info where no texture was projected
	}

	return;
}

void Syntheser::simpleViewSelector(UInt o) {

	Int bestView;
	Double maxDist, dist;
	Double ix, iy, iz;

	Double ox = m_yuvOutput[o]->m_currentCamParams->m_mExtMat.at(0, 3);
	Double oy = m_yuvOutput[o]->m_currentCamParams->m_mExtMat.at(1, 3);
	Double oz = m_yuvOutput[o]->m_currentCamParams->m_mExtMat.at(2, 3);

	for (UInt i = 0; i < m_uiNumberOfInputViews; i++) m_yuvInput[i]->m_uiPriority = 2;

	for (UInt s = 0; s < m_uiPrioritization; s++) {
		maxDist = 0;
		for (UInt i = 0; i < m_uiNumberOfInputViews; i++) {
			if (m_yuvInput[i]->m_uiPriority != 2) continue;

			ix = m_yuvInput[i]->m_currentCamParams->m_mExtMat.at(0, 3);
			iy = m_yuvInput[i]->m_currentCamParams->m_mExtMat.at(1, 3);
			iz = m_yuvInput[i]->m_currentCamParams->m_mExtMat.at(2, 3);

			dist = sqrt((ox - ix) * (ox - ix) + (oy - iy) * (oy - iy) + (oz - iz) * (oz - iz));

			if (dist > maxDist) {
				maxDist = dist;
				bestView = i;
			}

		}

		m_yuvInput[bestView]->m_uiPriority = 1;
	}

	m_uiMinPriority = 1;
	m_uiMaxPriority = 2;

	return;
}

void Syntheser::blendTextures(UInt o) {

	for (UInt p = m_uiMinPriority; p <= m_uiMaxPriority; p++) {
		for (UInt h = 0, pp = 0; h < m_yuvOutput[o]->m_uiHeight; h++) {
			for (UInt w = 0; w < m_yuvOutput[o]->m_uiWidth; w++, pp++) {

				if (m_uiDepthBasedPrioritization) {
					if (p > m_uiMinPriority) {
						if (m_yuvOutput[o][0].m_afYUVD[3][pp] != 0 && m_yuvOutput[o][0].m_afYUVD[3][pp] - m_fDepthBlendingThreshold < m_yuvIntermediate[o][1][0].m_afYUVD[3][pp]) continue;
						for (UInt v = 0; v < 4; v++) m_yuvOutput[o][0].m_afYUVD[v][pp] = 0;
					}
				}
				else {
					if (m_yuvOutput[o][0].m_afYUVD[3][pp] != 0) continue;
				}

				Float farthestDepth = BIG_FLOAT_VALUE;
				Bool projectedFromAnyInput = false;

				for (UInt i = 0; i < m_uiNumberOfInputViews; i++) {
					if (m_yuvInput[i]->m_uiPriority != p || m_yuvIntermediate[o][i][0].m_afYUVD[3][pp] == 0) continue;
					projectedFromAnyInput = true;
					if (farthestDepth > m_yuvIntermediate[o][i][0].m_afYUVD[3][pp]) {
						farthestDepth = m_yuvIntermediate[o][i][0].m_afYUVD[3][pp];
					}
				}

				if (!projectedFromAnyInput) continue;

				Float weight, diffX, diffY, diffZ;
				Float sumOfSquares;
				Float sumOfWeights = 0.0;

				for (UInt i = 0; i < m_uiNumberOfInputViews; i++) {
					if (m_yuvInput[i]->m_uiPriority != p || m_yuvIntermediate[o][i][0].m_afYUVD[3][pp] == 0) continue;

					if (farthestDepth + m_fDepthBlendingThreshold > m_yuvIntermediate[o][i][0].m_afYUVD[3][pp] && m_yuvIntermediate[o][i][0].m_afYUVD[3][pp] >= farthestDepth) { //do usuwania zlych glebi

						diffX = m_yuvOutput[o]->m_currentCamParams->m_mCvtdExtMat.at(0, 3) - m_yuvInput[i]->m_currentCamParams->m_mCvtdExtMat.at(0, 3);
						diffY = m_yuvOutput[o]->m_currentCamParams->m_mCvtdExtMat.at(1, 3) - m_yuvInput[i]->m_currentCamParams->m_mCvtdExtMat.at(1, 3);
						diffZ = m_yuvOutput[o]->m_currentCamParams->m_mCvtdExtMat.at(2, 3) - m_yuvInput[i]->m_currentCamParams->m_mCvtdExtMat.at(2, 3);

						sumOfSquares = diffX * diffX + diffY * diffY + diffZ * diffZ;

						if (sumOfSquares == 0) weight = 1000;
						else weight = 1.0 / sqrt(sumOfSquares);
						sumOfWeights += weight;

						for (UInt v = 0; v < 4; v++) m_yuvOutput[o][0].m_afYUVD[v][pp] += (m_yuvIntermediate[o][i][0].m_afYUVD[v][pp] * weight);
					}
				}

				for (UInt v = 0; v < 4; v++) m_yuvOutput[o][0].m_afYUVD[v][pp] /= sumOfWeights;

			}//w
		}//h
	}//p

	return;
}

void Syntheser::perspectiveInpainting(UInt o) {

	Int** inpaintingLUT = new Int * [2];
	for (UInt d = 0; d < 2; d++) inpaintingLUT[d] = new Int[m_yuvOutput[o]->m_uiDepthLumaFrameSizeInPixels];

	Int H = m_yuvOutput[o]->m_uiHeight;
	Int W = m_yuvOutput[o]->m_uiWidth;

	//analysis from top-left

	for (Int h = 0; h < H; h++) {
		Int hW = h * W;
		for (Int w = 0; w < W; w++) {
			Int pp = hW + w;

			inpaintingLUT[0][pp] = pp;

			if (m_yuvOutput[o][0].m_afYUVD[3][pp] == 0 || (m_yuvOutput[o][0].m_afYUVD[0][pp] == 0 && m_yuvOutput[o][0].m_afYUVD[1][pp] == 0 && m_yuvOutput[o][0].m_afYUVD[2][pp] == 0)) {

				if (w > 0) inpaintingLUT[0][pp] = inpaintingLUT[0][pp - 1];
				else inpaintingLUT[0][pp] = -1;
			}

		}//w
	}//h

	//analysis from bottom-right

	for (Int h = H - 1; h >= 0; h--) {
		Int hW = h * W;
		for (Int w = W - 1; w >= 0; w--) {
			Int pp = hW + w;

			inpaintingLUT[1][pp] = pp;

			if (m_yuvOutput[o][0].m_afYUVD[3][pp] == 0 || (m_yuvOutput[o][0].m_afYUVD[0][pp] == 0 && m_yuvOutput[o][0].m_afYUVD[1][pp] == 0 && m_yuvOutput[o][0].m_afYUVD[2][pp] == 0)) {

				if (w < W - 1) inpaintingLUT[1][pp] = inpaintingLUT[1][pp + 1];
				else inpaintingLUT[1][pp] = -1;
			}

		}//w
	}//h

	//inpainting

	for (Int h = 0, pp = 0; h < H; h++) {
		for (Int w = 0; w < W; w++, pp++) {

			if (m_yuvOutput[o][0].m_afYUVD[3][pp] != 0) continue;

			Float sumWeight = 0.0;
			Int dist;
			Float weight;
			Float closestDepth = 0;

			for (UInt d = 0; d < 2; d++) { //farthest depth searching
				if (inpaintingLUT[d][pp] == -1) continue;
				if (closestDepth < m_yuvOutput[o][0].m_afYUVD[3][inpaintingLUT[d][pp]]) closestDepth = m_yuvOutput[o][0].m_afYUVD[3][inpaintingLUT[d][pp]];
			}

			for (UInt d = 0; d < 2; d++) {
				if (inpaintingLUT[d][pp] == -1) continue;

				if (d < 2) dist = abs(Int(pp) - inpaintingLUT[d][pp]);
				else dist = abs(Int(pp) - inpaintingLUT[d][pp]) / W;

				if (m_yuvOutput[o][0].m_afYUVD[3][inpaintingLUT[d][pp]] + closestDepth < m_fDepthBlendingThreshold) continue;

				weight = 1.0 / (dist);

				m_yuvOutput[o][0].m_afYUVD[0][pp] += (m_yuvOutput[o][0].m_afYUVD[0][inpaintingLUT[d][pp]] * weight);
				m_yuvOutput[o][0].m_afYUVD[1][pp] += (m_yuvOutput[o][0].m_afYUVD[1][inpaintingLUT[d][pp]] * weight);
				m_yuvOutput[o][0].m_afYUVD[2][pp] += (m_yuvOutput[o][0].m_afYUVD[2][inpaintingLUT[d][pp]] * weight);

				m_yuvOutput[o][0].m_afYUVD[3][pp] += (m_yuvOutput[o][0].m_afYUVD[3][inpaintingLUT[d][pp]] * weight);

				sumWeight += weight;
			}

			if (sumWeight > 0) {
				m_yuvOutput[o][0].m_afYUVD[0][pp] /= sumWeight;
				m_yuvOutput[o][0].m_afYUVD[1][pp] /= sumWeight;
				m_yuvOutput[o][0].m_afYUVD[2][pp] /= sumWeight;

				m_yuvOutput[o][0].m_afYUVD[3][pp] /= sumWeight;

			}

		}//w
	}//h

	//analysis from top-left

	for (Int h = 0; h < H; h++) {
		Int hW = h * W;
		for (Int w = 0; w < W; w++) {
			Int pp = hW + w;

			inpaintingLUT[0][pp] = pp;

			if (m_yuvOutput[o][0].m_afYUVD[3][pp] == 0) {

				if (h > 0) inpaintingLUT[0][pp] = inpaintingLUT[0][pp - W];
				else inpaintingLUT[0][pp] = -1;
			}

		}//w
	}//h

	//analysis from bottom-right

	for (Int h = H - 1; h >= 0; h--) {
		Int hW = h * W;
		for (Int w = W - 1; w >= 0; w--) {
			Int pp = hW + w;

			inpaintingLUT[1][pp] = pp;

			if (m_yuvOutput[o][0].m_afYUVD[3][pp] == 0) {

				if (h < H - 1) inpaintingLUT[1][pp] = inpaintingLUT[1][pp + W];
				else inpaintingLUT[1][pp] = -1;
			}

		}//w
	}//h

	//inpainting

	for (Int h = 0, pp = 0; h < H; h++) {
		for (Int w = 0; w < W; w++, pp++) {

			if (m_yuvOutput[o][0].m_afYUVD[3][pp] != 0) continue;

			Float sumWeight = 0.0;
			Int dist;
			Float weight;
			Float closestDepth = 0;

			for (UInt d = 0; d < 2; d++) { //farthest depth searching
				if (inpaintingLUT[d][pp] == -1) continue;
				if (closestDepth < m_yuvOutput[o][0].m_afYUVD[3][inpaintingLUT[d][pp]]) closestDepth = m_yuvOutput[o][0].m_afYUVD[3][inpaintingLUT[d][pp]];
			}

			for (UInt d = 0; d < 2; d++) {
				if (inpaintingLUT[d][pp] == -1) continue;

				if (d < 2) dist = abs(Int(pp) - inpaintingLUT[d][pp]);
				else dist = abs(Int(pp) - inpaintingLUT[d][pp]) / W;

				if (m_yuvOutput[o][0].m_afYUVD[3][inpaintingLUT[d][pp]] + closestDepth < m_fDepthBlendingThreshold) continue;

				weight = 1.0 / (dist);

				m_yuvOutput[o][0].m_afYUVD[0][pp] += (m_yuvOutput[o][0].m_afYUVD[0][inpaintingLUT[d][pp]] * weight);
				m_yuvOutput[o][0].m_afYUVD[1][pp] += (m_yuvOutput[o][0].m_afYUVD[1][inpaintingLUT[d][pp]] * weight);
				m_yuvOutput[o][0].m_afYUVD[2][pp] += (m_yuvOutput[o][0].m_afYUVD[2][inpaintingLUT[d][pp]] * weight);

				m_yuvOutput[o][0].m_afYUVD[3][pp] += (m_yuvOutput[o][0].m_afYUVD[3][inpaintingLUT[d][pp]] * weight);

				sumWeight += weight;
			}

			if (sumWeight > 0) {
				m_yuvOutput[o][0].m_afYUVD[0][pp] /= sumWeight;
				m_yuvOutput[o][0].m_afYUVD[1][pp] /= sumWeight;
				m_yuvOutput[o][0].m_afYUVD[2][pp] /= sumWeight;

				m_yuvOutput[o][0].m_afYUVD[3][pp] /= sumWeight;

			}

		}//w
	}//h

	for (UInt d = 0; d < 2; d++) delete inpaintingLUT[d];
	delete inpaintingLUT;

	return;
}

void Syntheser::inp2(UInt o) {

	Int M = INP2_SIZE;

	Int H = m_yuvOutput[o]->m_uiHeight;
	Int W = m_yuvOutput[o]->m_uiWidth;

	for (Int m = 1; m <= M; m++) {
		Int mh = m;
		Int mw = m;
		for (Int h = mh; h < H - mh; h++) {
			Int hW = h * W;
			Int mhW = mh * W;
			for (Int w = mw; w < W - mw; w++) {
				Int pp = hW + w;

				if (m_yuvOutput[o][0].m_afYUVD[3][pp] != 0 && m_yuvOutput[o][0].m_afYUVD[3][pp] < m_yuvOutput[o][0].m_afYUVD[3][pp - mw] - m_fDepthBlendingThreshold && m_yuvOutput[o][0].m_afYUVD[3][pp] < m_yuvOutput[o][0].m_afYUVD[3][pp + mw] - m_fDepthBlendingThreshold) {
					if (abs(m_yuvOutput[o][0].m_afYUVD[3][pp - mw] - m_yuvOutput[o][0].m_afYUVD[3][pp + mw]) > m_fDepthBlendingThreshold) continue;
					for (UInt c = 0; c < 4; c++) {
						m_yuvOutput[o][0].m_afYUVD[c][pp] = (m_yuvOutput[o][0].m_afYUVD[c][pp - mw] + m_yuvOutput[o][0].m_afYUVD[c][pp + mw]) / 2;
					}
				}
				else if (m_yuvOutput[o][0].m_afYUVD[3][pp] != 0 && m_yuvOutput[o][0].m_afYUVD[3][pp] < m_yuvOutput[o][0].m_afYUVD[3][pp - mhW] - m_fDepthBlendingThreshold && m_yuvOutput[o][0].m_afYUVD[3][pp] < m_yuvOutput[o][0].m_afYUVD[3][pp + mhW] - m_fDepthBlendingThreshold) {
					if (abs(m_yuvOutput[o][0].m_afYUVD[3][pp - mhW] - m_yuvOutput[o][0].m_afYUVD[3][pp + mhW]) > m_fDepthBlendingThreshold) continue;
					for (UInt c = 0; c < 4; c++) {
						m_yuvOutput[o][0].m_afYUVD[c][pp] = (m_yuvOutput[o][0].m_afYUVD[c][pp - mhW] + m_yuvOutput[o][0].m_afYUVD[c][pp + mhW]) / 2;
					}
				}

			}
		}
	}

	return;
}

void Syntheser::inpaintDisocclusions(UInt o) {

	inp2(o);

#if AVERAGED_INPAINTING
	Int W = m_yuvOutput[o]->m_uiWidth;
	Int H = m_yuvOutput[o]->m_uiHeight;

	Float** tmpAvgInp = new Float * [4];
	for (UInt c = 0; c < 4; c++) {
		tmpAvgInp[c] = new Float[W * H];
		for (Int h = 0, pp = 0; h < H; h++) {
			for (Int w = 0; w < W; w++, pp++) {
				tmpAvgInp[c][pp] = m_yuvOutput[o][0].m_afYUVD[c][pp];
			}//w
		}//h
	}//c
#endif

	if (m_yuvOutput[o]->m_uiFormat == 360) omniInpainting(o);
	else perspectiveInpainting(o);

#if 3
	for (Int h = 0, pp = 0; h < H; h++) {
		for (Int w = 0; w < W; w++, pp++) {

			if (tmpAvgInp[3][pp] == 0) {

				for (UInt c = 0; c < 4; c++) {
					Int cnt = 0;
					Float sum = 0;

					for (Int hh = Max(0, h - AVERAGED_INPAINTING); hh <= Min(H - 1, h + AVERAGED_INPAINTING); hh++) {
						UInt ppp = hh * W + w;
						sum += m_yuvOutput[o][0].m_afYUVD[c][ppp];
						cnt++;
					}//h
					tmpAvgInp[c][pp] = sum / cnt;
				}//c

			}//if
		}//w
	}//h

	for (Int h = 0, pp = 0; h < H; h++) {
		for (Int w = 0; w < W; w++, pp++) {
			for (UInt c = 0; c < 4; c++) {
				m_yuvOutput[o][0].m_afYUVD[c][pp] = tmpAvgInp[c][pp];
			}//c
		}//w
	}//h

	for (UInt c = 0; c < 4; c++) delete tmpAvgInp[c];
	delete tmpAvgInp;
#endif

	return;
}

void Syntheser::omniInpainting(UInt o) {

	Int H = m_yuvOutput[o]->m_uiHeight;
	Int W = m_yuvOutput[o]->m_uiWidth;

	Int mapSize = 5; //0: pp of pix in orig image, 1: is pixel hole?, 2: left neighbor's pp in orig image, 3: right neighbor's pp in orig image, 4: inverse mapping
	Int** map = new Int * [mapSize];
	for (Int i = 0; i < mapSize; i++) {
		map[i] = new Int[H * W];
	}

	Int centralY = H / 2;
	Int oldPP, oldH, oldW;
	Int centralX = W / 2;
	Double newH, newW;
	Int newPP;

	for (Int h = 0; h < H; h++) {
		for (Int w = 0; w < W; w++) {
			Int pp = h * W + w;
			map[0][pp] = -1;
			map[1][pp] = 1;
			map[2][pp] = -1;
			map[3][pp] = -1;
			map[4][pp] = -1;
		}
	}

	Int W2 = W / 2;
	Int H2 = H / 2;
	Double tmpH, tmpW;
	Double angleRange = m_yuvOutput[o]->m_dAovW / PI2;

	for (Int h = 0; h < H; h++) {
		oldH = h - centralY;
		for (Int w = 0; w < W; w++) {
			oldPP = h * W + w;

			oldW = w - centralX;
			tmpH = sqrt(H * h - h * h);
			if (tmpH / H2 > angleRange) tmpH = H2 * angleRange;
			newW = oldW * tmpH / H2;
			newW += centralX;
			tmpW = sqrt(W * newW - newW * newW);
			newH = oldH * W2 / tmpW;
			newH += centralY;

			if (Int(newH + 0.5) < 0 || Int(newH + 0.5) >= H) continue;

			newPP = Int(newH + 0.5) * W + Int(newW + 0.5);
			map[4][oldPP] = newPP;

			if (map[1][newPP] == 1) {
				map[0][newPP] = oldPP;
			}

			if (m_yuvOutput[o][0].m_afYUVD[3][oldPP] != 0) map[1][newPP] = 0; //1 if hole
		}
	}

	//analysis from top-left

	for (Int h = 0; h < H; h++) {
		Int hW = h * W;
		for (Int w = 0; w < W; w++) {
			Int pp = hW + w;

			map[2][pp] = map[0][pp];

			if (map[1][pp]) {

				if (w > 0) map[2][pp] = map[2][pp - 1];
				else map[2][pp] = -1;

			}

		}//w
	}//h

	//analysis from bottom-right

	for (Int h = H - 1; h >= 0; h--) {
		Int hW = h * W;
		for (Int w = W - 1; w >= 0; w--) {
			Int pp = hW + w;

			map[3][pp] = map[0][pp];

			if (map[1][pp]) {

				if (w < W - 1) map[3][pp] = map[3][pp + 1];
				else map[3][pp] = -1;
			}

		}//w
	}//h

	//inpainting

	Double dist2, dist3, sumdist;
	Double weight2, weight3;
	Int h2, w2, h3, w3;

	for (Int h = 0, pp = 0; h < H; h++) {
		for (Int w = 0; w < W; w++, pp++) {

			if (m_yuvOutput[o][0].m_afYUVD[3][pp] == 0 && map[4][pp] != -1) {
				if (map[2][map[4][pp]] != -1 && map[3][map[4][pp]] != -1) {
					if (m_yuvOutput[o][0].m_afYUVD[3][map[2][map[4][pp]]] < m_yuvOutput[o][0].m_afYUVD[3][map[3][map[4][pp]]] - m_fDepthBlendingThreshold) { //3 dalej
						for (UInt c = 0; c < 4; c++) {
							m_yuvOutput[o][0].m_afYUVD[c][pp] = m_yuvOutput[o][0].m_afYUVD[c][map[3][map[4][pp]]];
						}
					}
					else if (m_yuvOutput[o][0].m_afYUVD[3][map[3][map[4][pp]]] < m_yuvOutput[o][0].m_afYUVD[3][map[2][map[4][pp]]] - m_fDepthBlendingThreshold) { //2 dalej
						for (UInt c = 0; c < 4; c++) {
							m_yuvOutput[o][0].m_afYUVD[c][pp] = m_yuvOutput[o][0].m_afYUVD[c][map[2][map[4][pp]]];
						}
					}
					else { //blend
						h2 = map[2][map[4][pp]] / W;
						h3 = map[3][map[4][pp]] / W;
						w2 = map[2][map[4][pp]] % W;
						w3 = map[3][map[4][pp]] % W;
						dist2 = sqrt((h - h2) * (h - h2) + (w - w2) * (w - w2));
						dist3 = sqrt((h - h3) * (h - h3) + (w - w3) * (w - w3));
						sumdist = dist2 + dist3;
						weight2 = dist3 / sumdist;
						weight3 = dist2 / sumdist;

						for (UInt c = 0; c < 4; c++) {
							m_yuvOutput[o][0].m_afYUVD[c][pp] = (m_yuvOutput[o][0].m_afYUVD[c][map[2][map[4][pp]]] * weight2 + m_yuvOutput[o][0].m_afYUVD[c][map[3][map[4][pp]]] * weight3);
						}
					}
				}
				else if (map[2][map[4][pp]] != -1) {
					for (UInt c = 0; c < 4; c++) {
						m_yuvOutput[o][0].m_afYUVD[c][pp] = m_yuvOutput[o][0].m_afYUVD[c][map[2][map[4][pp]]];
					}
				}
				else if (map[3][map[4][pp]] != -1) {
					for (UInt c = 0; c < 4; c++) {
						m_yuvOutput[o][0].m_afYUVD[c][pp] = m_yuvOutput[o][0].m_afYUVD[c][map[3][map[4][pp]]];
					}
				}
			}

		}
	}

	for (Int h = 0; h < H; h++) {
		Int hW = h * W;
		for (Int w = 0; w < W; w++) {
			Int pp = hW + w;
			if (m_yuvOutput[o][0].m_afYUVD[3][pp] == 0) {
				if (w > 0)
					for (UInt c = 0; c < 4; c++)
						m_yuvOutput[o][0].m_afYUVD[c][pp] = m_yuvOutput[o][0].m_afYUVD[c][pp - 1];
				else if (h > 0)
					for (UInt c = 0; c < 4; c++)
						m_yuvOutput[o][0].m_afYUVD[c][pp] = m_yuvOutput[o][0].m_afYUVD[c][pp - W];
			}
		}
	}

	for (Int h = H - 1; h >= 0; h--) {
		Int hW = h * W;
		for (Int w = W - 1; w >= 0; w--) {
			Int pp = hW + w;
			if (m_yuvOutput[o][0].m_afYUVD[3][pp] == 0) {
				if (w < W - 1)
					for (UInt c = 0; c < 4; c++)
						m_yuvOutput[o][0].m_afYUVD[c][pp] = m_yuvOutput[o][0].m_afYUVD[c][pp + 1];
				else if (h < H - 1)
					for (UInt c = 0; c < 4; c++)
						m_yuvOutput[o][0].m_afYUVD[c][pp] = m_yuvOutput[o][0].m_afYUVD[c][pp + W];
			}
		}
	}

	for (UInt m = 0; m < mapSize; m++) delete map[m];
	delete map;

	return;
}

void Syntheser::loadInputViews(UInt frame) {

	for (UInt i = 0; i < m_uiNumberOfInputViews; i++) {

		m_yuvInput[i]->readViewFrame(frame);
		m_yuvInput[i]->readDepthFrame(frame);

	}

	return;
}

void Syntheser::saveOutputViews(Bool append) {

	for (UInt o = 0; o < m_uiNumberOfOutputViews; o++) {
		m_yuvOutput[o][0].writeViewFrame(append);
		m_yuvOutput[o][0].writeDepthFrame(append);
	}

	return;
}

template <typename T>
void Syntheser::qSort(T tab[], Int left, Int right) {

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

void Syntheser::outputIntermediates(UInt i, UInt o) {

	String sTex = "test_" + std::to_string(m_yuvIntermediate[o][i]->m_uiWidth) + "x" + std::to_string(m_yuvIntermediate[o][i]->m_uiHeight) + "_10bps.yuv";
	String sDepth = "test_" + std::to_string(m_yuvIntermediate[o][i]->m_uiWidth) + "x" + std::to_string(m_yuvIntermediate[o][i]->m_uiHeight) + "_10bps_depth.yuv";

	m_yuvIntermediate[o][i]->enableProcessingArraysOutputting(sTex, sDepth, m_yuvInput[i]->m_dZNear, m_yuvInput[i]->m_dZFar);

	m_yuvIntermediate[o][i]->cvtProcessingArraysToOutput();
	m_yuvIntermediate[o][i]->writeDepthFrame(1);
	m_yuvIntermediate[o][i]->writeViewFrame(1);

	return;
}

void Syntheser::synthesize(UInt frame) {

	clearProcessingArrays();
	cvtInputArraysToProcessing();

	if (m_uiRemoveGhostEdges) removeGhostEdges();

	for (UInt o = 0; o < m_uiNumberOfOutputViews; o++) {

		UInt outFormat = m_yuvOutput[o]->m_uiFormat;

		m_yuvOutput[o]->m_sCurrentCameraName = m_yuvOutput[o]->m_asCameraNames[frame % m_yuvOutput[o]->m_asCameraNames.size()];
		m_yuvOutput[o]->m_currentCamParams = m_yuvOutput[o]->m_aCamParams[frame % m_yuvOutput[o]->m_aCamParams.size()];

		for (UInt i = 0; i < m_uiNumberOfInputViews; i++) {

			UInt inFormat = m_yuvInput[i]->m_uiFormat;

			if (inFormat == 0 && outFormat == 0) { projectDepth<0, 0>(i, o); 
			filterDepth(i, o); 
			projectTexture<0, 0>(i, o); }
			if (inFormat == 0 && outFormat != 0) { projectDepth<0, 1>(i, o); 
			filterDepth(i, o); 
			projectTexture<0, 1>(i, o); }
			if (inFormat != 0 && outFormat == 0) { projectDepth<1, 0>(i, o); 
			filterDepth(i, o); 
			projectTexture<1, 0>(i, o); }
			if (inFormat != 0 && outFormat != 0) { projectDepth<1, 1>(i, o); 
			filterDepth(i, o); 
			projectTexture<1, 1>(i, o); }

#if OUTPUT_INTERMEDIATES			
			outputIntermediates(i, o);
#endif
		}//i

		if (m_uiPrioritization) simpleViewSelector(o);

		blendTextures(o);

#if ENABLE_INPAINTING
		inpaintDisocclusions(o);
#endif
	}//o

	cvtProcessingArraysToOutput();

	return;
}

