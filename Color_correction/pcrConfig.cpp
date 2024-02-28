#include "pcrConfig.h"

Config::Config() :
	m_uiNumberOfInputViews(0),
	m_uiNumberOfOutputViews(0),
	m_uiNumberOfFrames(0),
	m_uiStartFrame(0),
	m_sRealCameraParameterFile(""),
	m_iWidth(0),
	m_iHeight(0),
	m_uiFormat(0),
	m_dAovL(-PI),
	m_dAovR(PI),
	m_dAovT(PI_HALF),
	m_dAovB(-PI_HALF),
	m_dZNear(0),
	m_dZFar(0),
	m_uiDepthChromaSubsampling(0),
	m_uiDepthBitsPerSample(0),
	m_uiViewChromaSubsampling(0),
	m_uiViewBitsPerSample(0),
	m_uiSynthesisMethod(1),
	m_fDepthBlendingThreshold(0.0),
	m_cfgInputs(NULL),
	m_cfgOutputs(NULL) {}

void Config::readCfgFile(const char* filename) {

	FILE* fileCfg = fopen(filename, "r");

	if (fileCfg == NULL) {
		fprintf(stdout, "Error: cfg file %s does not exist\n", filename);
		return;
	}

	const int MAX_TEXT_LENGTH = 200;
	Char tmpPar[MAX_TEXT_LENGTH];
	Char tmpVal[MAX_TEXT_LENGTH];

	//common parameters reading
	fseek(fileCfg, 0, 0);
	while (!feof(fileCfg)) {

		fscanf(fileCfg, "%s", tmpPar);

		if (feof(fileCfg)) break;
		if (tmpPar[0] == '#') fgets(tmpPar, MAX_TEXT_LENGTH, fileCfg);
		else {
			fscanf(fileCfg, "%s", tmpVal);

			if (!strcmp(tmpPar, "NumberOfInputViews")) m_uiNumberOfInputViews = atoi(tmpVal);
			if (!strcmp(tmpPar, "NumberOfOutputViews")) m_uiNumberOfOutputViews = atoi(tmpVal);

			if (!strcmp(tmpPar, "NumberOfFrames")) m_uiNumberOfFrames = atoi(tmpVal);
			if (!strcmp(tmpPar, "StartFrame")) m_uiStartFrame = atoi(tmpVal);

			if (!strcmp(tmpPar, "RealCameraParameterFile")) m_sRealCameraParameterFile = tmpVal;

			if (!strcmp(tmpPar, "Width")) m_iWidth = atoi(tmpVal);
			if (!strcmp(tmpPar, "Height")) m_iHeight = atoi(tmpVal);

			if (!strcmp(tmpPar, "Format")) {
				if (!strcmp(tmpVal, "Perspective")) m_uiFormat = 0;
				if (!strncmp(tmpVal, "Omni", 4)) m_uiFormat = 360;
			}

			if (!strcmp(tmpPar, "AOV_Left")) m_dAovL = atof(tmpVal) * PI / 180.0;
			if (!strcmp(tmpPar, "AOV_Right")) m_dAovR = atof(tmpVal) * PI / 180.0;
			if (!strcmp(tmpPar, "AOV_Top")) m_dAovT = atof(tmpVal) * PI / 180.0;
			if (!strcmp(tmpPar, "AOV_Bottom")) m_dAovB = atof(tmpVal) * PI / 180.0;

			if (!strcmp(tmpPar, "ZNear")) m_dZNear = atof(tmpVal);
			if (!strcmp(tmpPar, "ZFar")) m_dZFar = atof(tmpVal);

			if (!strcmp(tmpPar, "DepthChromaSubsampling")) m_uiDepthChromaSubsampling = atoi(tmpVal);
			if (!strcmp(tmpPar, "DepthBitsPerSample")) m_uiDepthBitsPerSample = atoi(tmpVal);

			if (!strcmp(tmpPar, "ViewChromaSubsampling")) m_uiViewChromaSubsampling = atoi(tmpVal);
			if (!strcmp(tmpPar, "ViewBitsPerSample")) m_uiViewBitsPerSample = atoi(tmpVal);

			//Synthesis parameters

			if (!strcmp(tmpPar, "SynthesisMethod")) m_uiSynthesisMethod = atoi(tmpVal);
			if (!strcmp(tmpPar, "DepthBlendingThreshold")) m_fDepthBlendingThreshold = atof(tmpVal);

			if (!strncmp(tmpPar, "Input", 5) || !strncmp(tmpPar, "Output", 6) || !strncmp(tmpPar, "Ref", 3)) {
				while (1) {
					fscanf(fileCfg, "%s", tmpPar);
					if (!strcmp(tmpPar, "}")) break;
				}
			}
		}//else
	}//while

	if (!m_uiNumberOfInputViews) {
		fprintf(stdout, "NumberOfInputViews should be greater than 0\n");
		return;
	}
	if (!m_uiNumberOfOutputViews) {
		fprintf(stdout, "NumberOfOutputViews should be greater than 0\n");
		return;
	}

	m_cfgInputs = new CfgYUVParams[m_uiNumberOfInputViews];
	m_cfgOutputs = new CfgYUVParams[m_uiNumberOfOutputViews + 1];

	//common parameters copying for inputs and outputs

	for (UInt i = 0;i < m_uiNumberOfInputViews; i++) {
		m_cfgInputs[i].m_bDepthFileWritingEnabled = false;
		m_cfgInputs[i].m_bViewFileWritingEnabled = false;
		m_cfgInputs[i].m_sCurrentCameraName = "";
		m_cfgInputs[i].m_asCameraNames.clear();
		m_cfgInputs[i].m_sDepthFilename = "";
		m_cfgInputs[i].m_sViewFilename = "";
		m_cfgInputs[i].m_iWidth = m_iWidth;
		m_cfgInputs[i].m_iHeight = m_iHeight;
		m_cfgInputs[i].m_uiFormat = m_uiFormat;
		m_cfgInputs[i].m_dAovL = m_dAovL;
		m_cfgInputs[i].m_dAovR = m_dAovR;
		m_cfgInputs[i].m_dAovT = m_dAovT;
		m_cfgInputs[i].m_dAovB = m_dAovB;
		m_cfgInputs[i].m_dZNear = m_dZNear;
		m_cfgInputs[i].m_dZFar = m_dZFar;
		m_cfgInputs[i].m_uiDepthChromaSubsampling = m_uiDepthChromaSubsampling;
		m_cfgInputs[i].m_uiDepthBitsPerSample = m_uiDepthBitsPerSample;
		m_cfgInputs[i].m_uiViewChromaSubsampling = m_uiViewChromaSubsampling;
		m_cfgInputs[i].m_uiViewBitsPerSample = m_uiViewBitsPerSample;
	}

	for (UInt o = 0;o < m_uiNumberOfOutputViews + 1; o++) {
		m_cfgOutputs[o].m_bDepthFileWritingEnabled = false;
		m_cfgOutputs[o].m_bViewFileWritingEnabled = false;
		m_cfgOutputs[o].m_sCurrentCameraName = "";
		m_cfgOutputs[o].m_asCameraNames.clear();
		m_cfgOutputs[o].m_sDepthFilename = "";
		m_cfgOutputs[o].m_sViewFilename = "";
		m_cfgOutputs[o].m_iWidth = m_iWidth;
		m_cfgOutputs[o].m_iHeight = m_iHeight;
		m_cfgOutputs[o].m_uiFormat = m_uiFormat;
		m_cfgOutputs[o].m_dAovL = m_dAovL;
		m_cfgOutputs[o].m_dAovR = m_dAovR;
		m_cfgOutputs[o].m_dAovT = m_dAovT;
		m_cfgOutputs[o].m_dAovB = m_dAovB;
		m_cfgOutputs[o].m_dZNear = m_dZNear;
		m_cfgOutputs[o].m_dZFar = m_dZFar;
		m_cfgOutputs[o].m_uiDepthChromaSubsampling = m_uiDepthChromaSubsampling;
		m_cfgOutputs[o].m_uiDepthBitsPerSample = m_uiDepthBitsPerSample;
		m_cfgOutputs[o].m_uiViewChromaSubsampling = m_uiViewChromaSubsampling;
		m_cfgOutputs[o].m_uiViewBitsPerSample = m_uiViewBitsPerSample;
	}

	//input and output parameters reading and overwriting

	UInt inputCnt = 0, outputCnt = 0;
	fseek(fileCfg, 0, 0);
	while (!feof(fileCfg)) {

		fscanf(fileCfg, "%s", tmpPar);

		if (feof(fileCfg)) break;
		if (tmpPar[0] == '#') fgets(tmpPar, MAX_TEXT_LENGTH, fileCfg);
		else {
			fscanf(fileCfg, "%s", tmpVal);

			if (!strncmp(tmpPar, "Input", 5) && inputCnt < m_uiNumberOfInputViews) {
				while (1) {
					fscanf(fileCfg, "%s", tmpPar);
					if (tmpPar[0] == '#' || tmpPar[0] == '{') fgets(tmpPar, MAX_TEXT_LENGTH, fileCfg);
					else {
						if (!strcmp(tmpPar, "}")) {
							inputCnt++;
							break;
						}
						fscanf(fileCfg, "%s", tmpVal);

						if (!strcmp(tmpPar, "CameraName")) m_cfgInputs[inputCnt].m_sCurrentCameraName = tmpVal;
						if (!strcmp(tmpPar, "Depth")) m_cfgInputs[inputCnt].m_sDepthFilename = tmpVal;
						if (!strcmp(tmpPar, "View")) m_cfgInputs[inputCnt].m_sViewFilename = tmpVal;

						if (!strcmp(tmpPar, "Width")) m_cfgInputs[inputCnt].m_iWidth = atoi(tmpVal);
						if (!strcmp(tmpPar, "Height")) m_cfgInputs[inputCnt].m_iHeight = atoi(tmpVal);

						if (!strcmp(tmpPar, "Format")) {
							if (!strcmp(tmpVal, "Perspective")) m_cfgInputs[inputCnt].m_uiFormat = 0;
							if (!strncmp(tmpVal, "Omni", 4)) m_cfgInputs[inputCnt].m_uiFormat = 360;
						}

						if (!strcmp(tmpPar, "AOV_Left")) m_cfgInputs[inputCnt].m_dAovL = atof(tmpVal) * PI / 180.0;
						if (!strcmp(tmpPar, "AOV_Right")) m_cfgInputs[inputCnt].m_dAovR = atof(tmpVal) * PI / 180.0;
						if (!strcmp(tmpPar, "AOV_Top")) m_cfgInputs[inputCnt].m_dAovT = atof(tmpVal) * PI / 180.0;
						if (!strcmp(tmpPar, "AOV_Bottom")) m_cfgInputs[inputCnt].m_dAovB = atof(tmpVal) * PI / 180.0;

						if (!strcmp(tmpPar, "ZNear")) m_cfgInputs[inputCnt].m_dZNear = atof(tmpVal);
						if (!strcmp(tmpPar, "ZFar")) m_cfgInputs[inputCnt].m_dZFar = atof(tmpVal);

						if (!strcmp(tmpPar, "DepthChromaSubsampling")) m_cfgInputs[inputCnt].m_uiDepthChromaSubsampling = atoi(tmpVal);
						if (!strcmp(tmpPar, "DepthBitsPerSample")) m_cfgInputs[inputCnt].m_uiDepthBitsPerSample = atoi(tmpVal);
						
						if (!strcmp(tmpPar, "ViewChromaSubsampling")) m_cfgInputs[inputCnt].m_uiViewChromaSubsampling = atoi(tmpVal);
						if (!strcmp(tmpPar, "ViewBitsPerSample")) m_cfgInputs[inputCnt].m_uiViewBitsPerSample = atoi(tmpVal);
					}//else					
				}//while
			}
			if (!strncmp(tmpPar, "Output", 6) && outputCnt < m_uiNumberOfOutputViews) {

				m_cfgOutputs[outputCnt].m_bViewFileWritingEnabled = true;

				while (1) {
					fscanf(fileCfg, "%s", tmpPar);
					if (tmpPar[0] == '#' || tmpPar[0] == '{') fgets(tmpPar, MAX_TEXT_LENGTH, fileCfg);
					else {
						if (!strcmp(tmpPar, "}")) {
							outputCnt++;
							break;
						}
						fscanf(fileCfg, "%s", tmpVal);

						if (!strcmp(tmpPar, "CameraName")) {
							if (tmpVal[0] == '[') {
								while (1) {
									fscanf(fileCfg, "%s", tmpVal);
									if (!strcmp(tmpVal, "]")) break;
									m_cfgOutputs[outputCnt].m_asCameraNames.push_back(tmpVal);
								}
							}
							else {
								m_cfgOutputs[outputCnt].m_sCurrentCameraName = tmpVal;
								m_cfgOutputs[outputCnt].m_asCameraNames.push_back(tmpVal);
							}
						}

						if (!strcmp(tmpPar, "Depth")) m_cfgOutputs[outputCnt].m_sDepthFilename = tmpVal;
						if (!strcmp(tmpPar, "View")) m_cfgOutputs[outputCnt].m_sViewFilename = tmpVal;

						if (!strcmp(tmpPar, "Width")) m_cfgOutputs[outputCnt].m_iWidth = atoi(tmpVal);
						if (!strcmp(tmpPar, "Height")) m_cfgOutputs[outputCnt].m_iHeight = atoi(tmpVal);

						if (!strcmp(tmpPar, "Format")) {
							if (!strcmp(tmpVal, "Perspective")) m_cfgOutputs[outputCnt].m_uiFormat = 0;
							if (!strncmp(tmpVal, "Omni", 4)) m_cfgOutputs[outputCnt].m_uiFormat = 360;
						}

						if (!strcmp(tmpPar, "AOV_Left")) m_cfgOutputs[outputCnt].m_dAovL = atof(tmpVal) * PI / 180.0;
						if (!strcmp(tmpPar, "AOV_Right")) m_cfgOutputs[outputCnt].m_dAovR = atof(tmpVal) * PI / 180.0;
						if (!strcmp(tmpPar, "AOV_Top")) m_cfgOutputs[outputCnt].m_dAovT = atof(tmpVal) * PI / 180.0;
						if (!strcmp(tmpPar, "AOV_Bottom")) m_cfgOutputs[outputCnt].m_dAovB = atof(tmpVal) * PI / 180.0;

						if (!strcmp(tmpPar, "ZNear")) m_cfgOutputs[outputCnt].m_dZNear = atof(tmpVal);
						if (!strcmp(tmpPar, "ZFar")) m_cfgOutputs[outputCnt].m_dZFar = atof(tmpVal);

						if (!strcmp(tmpPar, "DepthChromaSubsampling")) m_cfgOutputs[outputCnt].m_uiDepthChromaSubsampling = atoi(tmpVal);
						if (!strcmp(tmpPar, "DepthBitsPerSample")) m_cfgOutputs[outputCnt].m_uiDepthBitsPerSample = atoi(tmpVal);

						if (!strcmp(tmpPar, "ViewChromaSubsampling")) m_cfgOutputs[outputCnt].m_uiViewChromaSubsampling = atoi(tmpVal);
						if (!strcmp(tmpPar, "ViewBitsPerSample")) m_cfgOutputs[outputCnt].m_uiViewBitsPerSample = atoi(tmpVal);
					}//else					
				}//while
			}
		}//else
	}//while	

	fclose(fileCfg);

	//camparam reading

	FILE* fileCamParam = fopen(m_sRealCameraParameterFile.c_str(), "r");

	if (fileCamParam == NULL) {
		fprintf(stdout, "Error: parameter file %s does not exist\n", m_sRealCameraParameterFile.c_str());
		return;
	}

	for (UInt i = 0;i < m_uiNumberOfInputViews;i++) {

		fseek(fileCamParam, 0, 0);
		while (!feof(fileCamParam)) {

			fscanf(fileCamParam, "%s", tmpPar);
			if (feof(fileCamParam)) break;

			if (!strcmp(tmpPar, m_cfgInputs[i].m_sCurrentCameraName.c_str())) {

				m_cfgInputs[i].m_currentCamParams = new CamParams();

				for (Int h = 0; h < 3; h++) {
					for (Int w = 0; w < 3; w++) {
						fscanf(fileCamParam, "%s", tmpVal);
						m_cfgInputs[i].m_currentCamParams->m_mIntMat.at(h, w) = atof(tmpVal);
					}
				}

				fscanf(fileCamParam, "%s", tmpVal);
				fscanf(fileCamParam, "%s", tmpVal);

				for (Int h = 0; h < 3; h++) {
					for (Int w = 0; w < 4; w++) {
						fscanf(fileCamParam, "%s", tmpVal);
						m_cfgInputs[i].m_currentCamParams->m_mExtMat.at(h, w) = atof(tmpVal);
					}
				}

				m_cfgInputs[i].m_currentCamParams->calc();

				break;
			}
		}
	}

	for (UInt o = 0;o < m_uiNumberOfOutputViews;o++) {
		for (UInt oo = 0;oo < m_cfgOutputs[o].m_asCameraNames.size();oo++) {

			fseek(fileCamParam, 0, 0);
			while (!feof(fileCamParam)) {

				fscanf(fileCamParam, "%s", tmpPar);
				if (feof(fileCamParam)) break;

				if (!strcmp(tmpPar, m_cfgOutputs[o].m_asCameraNames[oo].c_str())) {

					CamParams *tmpParams = new CamParams();

					for (Int h = 0; h < 3; h++) {
						for (Int w = 0; w < 3; w++) {
							fscanf(fileCamParam, "%s", tmpVal);
							tmpParams->m_mIntMat.at(h, w) = atof(tmpVal);
						}
					}

					fscanf(fileCamParam, "%s", tmpVal);
					fscanf(fileCamParam, "%s", tmpVal);

					for (Int h = 0; h < 3; h++) {
						for (Int w = 0; w < 4; w++) {
							fscanf(fileCamParam, "%s", tmpVal);
							tmpParams->m_mExtMat.at(h, w) = atof(tmpVal);
						}
					}

					tmpParams->calc();
					m_cfgOutputs[o].m_aCamParams.push_back(tmpParams);

					break;
				}
			}
		}
		m_cfgOutputs[o].m_currentCamParams = m_cfgOutputs[o].m_aCamParams[0];
		m_cfgOutputs[o].m_sCurrentCameraName = m_cfgOutputs[o].m_asCameraNames[0];
	}	

	fclose(fileCamParam);

	return;
}
