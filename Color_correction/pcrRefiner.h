#ifndef __PCR_REFINER_H__
#define __PCR_REFINER_H__

#include "TypeDef.h"
#include "pcrYUV.h"
#include "pcrConfig.h"
#include <math.h>

#include <iostream>

#if ESTIMATE_TIME
#include <time.h>
#endif

#define WEIGHT_IIR			0.5
#define BACKGROUND_WH_STEP	16
#define BACKGROUND_T_STEP	1

class Refiner {
public:
	UInt m_uiNumberOfInputViews;
	UInt m_uiNumberOfOutputViews;

	cArray<YUV*> m_yuvInput;
	cArray<YUV*> m_yuvOutput;

	cArray<cArray<YUV*>> m_yuvIntermediate;

	UInt m_uiSynthesisMethod;
	Double m_fDepthBlendingThreshold;

	cArray<Double> *m_IIR;
	Double m_dWeightIIR;

	Double ****m_yuvBackgroundVector;
	Double ***m_yuvBackground;
	Int m_iBackgroundVectorSize;

	~Refiner();
	Refiner(cArray<YUV*> yuvIn, cArray<YUV*> yuvOut, Config *cfg);

	void refine(UInt frame);

	void inpaintDisocclusions8(UInt o);
	void inpaintSmallAreas(UInt o);

	void loadInputDepthMaps(UInt frame);
	void saveOutputDepthMaps(Bool append);

	void loadInputViews(UInt frame);
	void saveOutputViews(Bool append);

	void clearProcessingArrays();

	void cvtInputArraysToProcessing();
	void cvtProcessingArraysToOutput();

	void calcImPosOmni(Int &W, Int &H, Double &wPos, Double &hPos, Double &invTargetZ, Matrix_3x1 *XYZ, Double &IAovL, Double &IAovW, Double &IAovB, Double &IAovH);
	void calcImPosPersp(Double &wPos, Double &hPos, Double &invTargetZ, Matrix_3x1 *XYZ, Matrix_4x4 *mat);
	void calcXYZPersp(Int &w, Int &h, Double &invZ, Matrix_3x1 *XYZ, Matrix_4x4 *mat);
	
	void projectDepth(UInt i, UInt o, UInt inFormat, UInt outFormat);

	void filterDepth(UInt i, UInt o);

	void projectTexture(UInt i, UInt o, UInt inFormat, UInt outFormat);
	void copyColorFromInput(UInt &i, UInt &o, Int &pp, Double &wPos, Double &hPos);

	void CE5_ColorRefinement(UInt i, UInt frame);
	void temphance(UInt i);

	void initBackgroundVector(UInt numberOfFrames);
	void fillBackgroundVector(UInt frame);
	void calcBackground();
	void deinitBackgroundVector();
	
	template <typename T>
	void qSort(T tab[], Int left, Int right);
};

#endif
