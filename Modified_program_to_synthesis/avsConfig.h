#ifndef __AVS_CONFIG_H__
#define __AVS_CONFIG_H__

#include "TypeDef.h"
#include "avsYUV.h"
#include <fstream>

class Config {
public:
	UInt m_uiNumberOfInputViews;
	UInt m_uiNumberOfOutputViews;	

	UInt m_uiNumberOfFrames;
	UInt m_uiStartFrame;

	String m_sRealCameraParameterFile;
	String m_sVirtualCameraParameterFile;

	//Common parameters
	UInt m_uiWidth;
	UInt m_uiHeight;

	UInt m_uiFormat; //0 - perspective, 360 - omnidirectional

	UInt m_uiPrioritization;
	UInt m_uiDepthBasedPrioritization;
	
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

	//Synthesis parameters
	Float m_fDepthBlendingThreshold;
	UInt m_uiRemoveGhostEdges;

	//Inputs
	CfgYUVParams *m_cfgInputs;

	//Outputs
	CfgYUVParams *m_cfgOutputs;

	void readCfgFile(const char* filename);
	Config();
};

#endif




