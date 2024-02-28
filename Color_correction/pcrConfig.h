#ifndef __PCR_CONFIG_H__
#define __PCR_CONFIG_H__

#include "TypeDef.h"
#include "pcrYUV.h"
#include <string.h>

class Config {
public:
	UInt m_uiNumberOfInputViews;
	UInt m_uiNumberOfOutputViews;

	UInt m_uiNumberOfFrames;
	UInt m_uiStartFrame;

	String m_sRealCameraParameterFile;

	//Common parameters
	Int m_iWidth;
	Int m_iHeight;

	UInt m_uiFormat; //0 - perspective, 360 - omnidirectional

	Double m_dAovL;
	Double m_dAovR;
	Double m_dAovT;
	Double m_dAovB;

	Double m_dZNear;
	Double m_dZFar;

	UInt m_uiDepthChromaSubsampling;
	UInt m_uiDepthBitsPerSample;

	UInt m_uiViewChromaSubsampling;
	UInt m_uiViewBitsPerSample;

	//Synthesis parameters
	UInt m_uiSynthesisMethod;
	Float m_fDepthBlendingThreshold;

	//Inputs
	CfgYUVParams *m_cfgInputs;

	//Outputs
	CfgYUVParams *m_cfgOutputs;

	void readCfgFile(const char* filename);
	Config();
};

#endif




