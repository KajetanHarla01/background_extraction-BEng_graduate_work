#ifndef __AVS_YUV_H__
#define __AVS_YUV_H__

#include "TypeDef.h"
#include "avsCamParams.h"

struct Pixel {
	UShort Y, U, V, D;
	Pixel(UShort y = 0, UShort u = 0, UShort v = 0, UShort d = 0) : Y(y), U(u), V(v), D(d) {};
};

struct CfgYUVParams {
	Bool m_bViewFileWritingEnabled;
	Bool m_bDepthFileWritingEnabled;

	cArray<String> m_asCameraNames;
	String m_sCurrentCameraName;

	String m_sViewFilename;
	String m_sDepthFilename;

	cArray<CamParams*> m_aCamParams;
	CamParams *m_currentCamParams;

	UInt m_uiWidth;
	UInt m_uiHeight;

	UInt m_uiFormat;

	Double m_dAovL;
	Double m_dAovR;
	Double m_dAovT;
	Double m_dAovB;

	Double m_dZNear;
	Double m_dZFar;

	UInt m_uiViewChromaSubsampling;
	UInt m_uiDepthChromaSubsampling;

	UInt m_uiViewBitsPerSample;
	UInt m_uiDepthBitsPerSample;

	UInt m_uiPriority;
};

class YUV {
public:
	Bool m_bViewFileWritingEnabled;
	Bool m_bDepthFileWritingEnabled;

	cArray<String> m_asCameraNames;
	String m_sCurrentCameraName;

	String m_sViewFilename;
	String m_sDepthFilename;

	cArray<CamParams*> m_aCamParams;
	CamParams *m_currentCamParams;

	UInt m_uiWidth; //both for Y and D
	UInt m_uiHeight;

	UInt m_uiFormat;

	Double m_dAovL;
	Double m_dAovR;
	Double m_dAovT;
	Double m_dAovB;
	
	Double m_dAovW;
	Double m_dAovH;

	UInt m_uiPriority;

	UInt m_uiViewChromaWidth;
	UInt m_uiViewChromaHeight;
	UInt m_uiDepthChromaWidth;
	UInt m_uiDepthChromaHeight;

	UInt m_uiViewChromaSubsampling;
	UInt m_uiDepthChromaSubsampling;

	UInt m_uiViewLumaFrameSizeInPixels;
	UInt m_uiViewLumaFrameSizeInBytes;
	UInt m_uiViewChromaFrameSizeInPixels;
	UInt m_uiViewChromaFrameSizeInBytes;

	UInt m_uiDepthLumaFrameSizeInPixels;
	UInt m_uiDepthLumaFrameSizeInBytes;	
	UInt m_uiDepthChromaFrameSizeInPixels;
	UInt m_uiDepthChromaFrameSizeInBytes;
	
	UInt m_uiViewBitsPerSample;
	UInt m_uiViewBytesPerSample;

	UInt m_uiDepthBitsPerSample;
	UInt m_uiDepthBytesPerSample;

	UInt m_uiMaxViewValue;
	Double m_dInvMaxViewValue;
	UInt m_uiMaxDepthValue;
	Double m_dInvMaxDepthValue;

	Double m_dZNear;
	Double m_dZFar;

	Double m_dInvZFar;
	Double m_dInvZNear;
	Double m_dInvZNearMinusInvZFar;
	Double m_dInvIZNMIZF;

	UChar *m_acY;
	UChar *m_acU;
	UChar *m_acV;
	UChar *m_acD;

	UShort *m_asY;
	UShort *m_asU;
	UShort *m_asV;
	UShort *m_asD;

	Int **m_aiYUVD;
	Float **m_afYUVD;

	~YUV();
	YUV();
	YUV(UInt w, UInt h);
	YUV(CfgYUVParams params);

	void initFromCfg(CfgYUVParams params);

	void initInOutArrays();
	void initProcessingArrays();

	void enableProcessingArraysOutputting(String outputViewName, String outputDepthName = "", Double znear = 0, Double zfar = 0);

	void cvtProcessingArraysToOutput();
	void cvtInputArraysToProcessing();
		
	void readViewFrame(UInt frame);
	void writeViewFrame(Bool append);	

	void readDepthFrame(UInt frame);
	void writeDepthFrame(Bool append);
};

#endif
