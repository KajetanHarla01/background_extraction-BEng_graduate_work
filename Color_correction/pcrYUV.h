#ifndef __PCR_YUV_H__
#define __PCR_YUV_H__

#include "TypeDef.h"
#include "pcrCamParams.h"
#include <string.h>
#include <iostream>

struct CfgYUVParams {
	Bool m_bDepthFileWritingEnabled;
	Bool m_bViewFileWritingEnabled;

	cArray<String> m_asCameraNames;
	String m_sCurrentCameraName;

	String m_sDepthFilename;
	String m_sViewFilename;

	cArray<CamParams*> m_aCamParams;
	CamParams *m_currentCamParams;

	Int m_iWidth;
	Int m_iHeight;

	UInt m_uiFormat;

	Double m_dAovL;
	Double m_dAovR;
	Double m_dAovT;
	Double m_dAovB;

	Float m_dZNear;
	Float m_dZFar;

	UInt m_uiDepthChromaSubsampling;
	UInt m_uiDepthBitsPerSample;
	UInt m_uiViewChromaSubsampling;
	UInt m_uiViewBitsPerSample;
};

class YUV {
public:
	Bool m_bDepthFileWritingEnabled;
	Bool m_bViewFileWritingEnabled;

	cArray<String> m_asCameraNames;
	String m_sCurrentCameraName;

	String m_sDepthFilename;
	String m_sViewFilename;

	cArray<CamParams*> m_aCamParams;
	CamParams *m_currentCamParams;

	Int m_iWidth;
	Int m_iHeight;

	UInt m_uiFormat;

	Double m_dAovL;
	Double m_dAovR;
	Double m_dAovT;
	Double m_dAovB;
	
	Double m_dAovW;
	Double m_dAovH;
	
	UInt m_uiDepthChromaWidth;
	UInt m_uiDepthChromaHeight;	
	UInt m_uiDepthChromaSubsampling;

	UInt m_uiViewChromaWidth;
	UInt m_uiViewChromaHeight;
	UInt m_uiViewChromaSubsampling;

	UInt m_uiDepthLumaFrameSizeInPixels;
	UInt m_uiDepthLumaFrameSizeInBytes;	
	UInt m_uiDepthChromaFrameSizeInPixels;
	UInt m_uiDepthChromaFrameSizeInBytes;

	UInt m_uiViewLumaFrameSizeInPixels;
	UInt m_uiViewLumaFrameSizeInBytes;
	UInt m_uiViewChromaFrameSizeInPixels;
	UInt m_uiViewChromaFrameSizeInBytes;

	UInt m_uiDepthBitsPerSample;
	UInt m_uiDepthBytesPerSample;

	UInt m_uiViewBitsPerSample;
	UInt m_uiViewBytesPerSample;

	UInt m_uiMaxDepthValue;
	Float m_dInvMaxDepthValue;

	UInt m_uiMaxViewValue;
	Float m_dInvMaxViewValue;

	Float m_dZNear;
	Float m_dZFar;

	Float m_dInvZFar;
	Float m_dInvZNear;
	Float m_dInvZNearMinusInvZFar;
	Float m_dInvIZNMIZF;

	UChar *m_acY, *m_acU, *m_acV, *m_acD;
	UShort *m_asY, *m_asU, *m_asV, *m_asD;
	Int **m_aiYUVD;
	Float **m_afYUVD;

	~YUV();
	YUV();
	YUV(UInt w, UInt h);
	YUV(CfgYUVParams params);

	void initFromCfg(CfgYUVParams params);

	void initInOutArrays();
	void initProcessingArrays();

	void enableProcessingArraysOutputting(String outputViewName = "", Double znear = 0, Double zfar = 0);

	void cvtProcessingArraysToOutput();
	void cvtInputArraysToProcessing();

	void readDepthFrame(UInt frame);
	void writeDepthFrame(Bool append);

	void readViewFrame(UInt frame);
	void writeViewFrame(Bool append);
};

#endif
