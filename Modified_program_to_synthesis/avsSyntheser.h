#ifndef __AVS_SYNTHESER_H__
#define __AVS_SYNTHESER_H__

#include "TypeDef.h"
#include "avsYUV.h"
#include "avsConfig.h"

#include <iostream>

class Syntheser {
public:
	UInt m_uiNumberOfInputViews;
	UInt m_uiNumberOfOutputViews;

	cArray<YUV*> m_yuvInput;
	cArray<YUV*> m_yuvOutput;
	
	cArray<cArray<YUV*>> m_yuvIntermediate;

	Double m_fDepthBlendingThreshold;
	UInt m_uiRemoveGhostEdges;

	UInt m_uiPrioritization;
	UInt m_uiDepthBasedPrioritization;

	UInt m_uiMinPriority;
	UInt m_uiMaxPriority;
	
	~Syntheser();
	Syntheser(cArray<YUV*> yuvIn, cArray<YUV*> yuvOut, Config *cfg);

	void synthesize(UInt frame);

	void loadInputViews(UInt frame);

	void saveOutputViews(Bool append);

	void clearProcessingArrays();

	void cvtInputArraysToProcessing();
	void cvtProcessingArraysToOutput();

	void removeGhostEdges();

	void calcImPosOmni(UInt &W, UInt &H, Double &wPos, Double &hPos, Double &invTargetZ, Matrix_3x1 *XYZ, Double &IAovL, Double &IAovW, Double &IAovB, Double &IAovH); //returns 1 if 2pi was added to pixel's position in virtual view; -1 if it was subtracted; 0 otherwise
	void calcImPosPersp(Double &wPos, Double &hPos, Double &invTargetZ, Matrix_3x1 *XYZ, Matrix_4x4 *mat);
	void calcXYZPersp(UInt &w, UInt &h, Double &invZ, Matrix_3x1 *XYZ, Matrix_4x4 *mat);
	
	template <UInt inFormat, UInt outFormat> void projectDepth(UInt i, UInt o);
	template <UInt inFormat, UInt outFormat> void projectTexture(UInt i, UInt o);

	void filterDepth(UInt i, UInt o);
	
	void copyColorFromInput(UInt &i, UInt &o, UInt &pp, Double &wPos, Double &hPos, Bool interpolate);

	void simpleViewSelector(UInt o);

	void blendTextures(UInt o);

	void inpaintDisocclusions(UInt o);
	void perspectiveInpainting(UInt o);

	void outputIntermediates(UInt i, UInt o);

	void omniInpainting(UInt o);
	
	void inp2(UInt o);

	template <typename T>
	void qSort(T tab[], Int left, Int right);

};

#endif
