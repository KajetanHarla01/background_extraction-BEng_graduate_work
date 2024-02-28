#include "avsYUV.h"

YUV::~YUV() {

	if (m_acY) { delete m_acY; m_acY = NULL; m_asY = NULL; }
	if (m_acU) { delete m_acU; m_acU = NULL; m_asU = NULL; }
	if (m_acV) { delete m_acV; m_acV = NULL; m_asV = NULL; }
	if (m_acD) { delete m_acD; m_acD = NULL; m_asD = NULL; }

	return;

}

YUV::YUV() {

	m_bViewFileWritingEnabled = false;
	m_bDepthFileWritingEnabled = false;

	m_acY = NULL; m_acU = NULL; m_acV = NULL; m_acD = NULL;
	m_asY = NULL; m_asU = NULL; m_asV = NULL; m_asD = NULL;

	return;
}

YUV::YUV(UInt w, UInt h) {

	YUV();

	m_uiWidth = w;
	m_uiHeight = h;
	m_uiViewLumaFrameSizeInPixels = m_uiWidth * m_uiHeight;

	initProcessingArrays();

	return;
}

void YUV::enableProcessingArraysOutputting(String outputViewName, String outputDepthName, Double znear, Double zfar) {

	m_bViewFileWritingEnabled = true;
	m_bDepthFileWritingEnabled = true;

	m_dZNear = znear;
	m_dZFar = zfar;

	m_uiViewChromaSubsampling = 420; //444
	m_uiDepthChromaSubsampling = 420; //400

	m_uiViewBitsPerSample = 10; //16
	m_uiViewBytesPerSample = 2;

	m_uiDepthBitsPerSample = 10; //16
	m_uiDepthBytesPerSample = 2;

	m_uiMaxViewValue = (1 << (m_uiViewBitsPerSample)) - 1; //255;
	m_uiMaxDepthValue = (1 << (m_uiDepthBitsPerSample)) - 1;//65535;

	printf("%i\n",m_uiMaxViewValue);

	m_dInvMaxDepthValue = 1.0 / m_uiMaxDepthValue;
	m_dInvMaxViewValue = 1.0 / m_uiMaxViewValue;

	m_dInvZNear = 1.0 / m_dZNear;
	m_dInvZFar = 1.0 / m_dZFar;
	m_dInvZNearMinusInvZFar = m_dInvZNear - m_dInvZFar;
	m_dInvIZNMIZF = 1.0 / m_dInvZNearMinusInvZFar;

	m_sViewFilename = outputViewName;
	m_sDepthFilename = outputDepthName;

	if (m_uiViewChromaSubsampling == 400) {
		m_uiViewChromaWidth = 0;
		m_uiViewChromaHeight = 0;
	}
	else if (m_uiViewChromaSubsampling == 420) {
		m_uiViewChromaWidth = m_uiWidth / 2;
		m_uiViewChromaHeight = m_uiHeight / 2;
	}
	else if (m_uiViewChromaSubsampling == 444) {
		m_uiViewChromaWidth = m_uiWidth;
		m_uiViewChromaHeight = m_uiHeight;
	}
	
	if (m_uiDepthChromaSubsampling == 400) {
		m_uiDepthChromaWidth = 0;
		m_uiDepthChromaHeight = 0;
	}		
	else if (m_uiDepthChromaSubsampling == 420) {
		m_uiDepthChromaWidth = m_uiWidth / 2;
		m_uiDepthChromaHeight = m_uiHeight / 2;
	}		
	else if (m_uiDepthChromaSubsampling == 444) {
		m_uiDepthChromaWidth = m_uiWidth;
		m_uiDepthChromaHeight = m_uiHeight;
	}

	m_uiViewChromaFrameSizeInPixels = m_uiViewChromaWidth * m_uiViewChromaHeight;
	m_uiDepthChromaFrameSizeInPixels = m_uiDepthChromaWidth * m_uiDepthChromaHeight;

	m_uiDepthLumaFrameSizeInPixels = m_uiViewLumaFrameSizeInPixels;

	m_uiViewLumaFrameSizeInBytes = m_uiViewLumaFrameSizeInPixels * m_uiViewBytesPerSample;
	m_uiDepthLumaFrameSizeInBytes = m_uiDepthLumaFrameSizeInPixels * m_uiDepthBytesPerSample;

	m_uiViewChromaFrameSizeInBytes = m_uiViewChromaFrameSizeInPixels * m_uiViewBytesPerSample;
	m_uiDepthChromaFrameSizeInBytes = m_uiDepthChromaFrameSizeInPixels * m_uiDepthBytesPerSample;

	initInOutArrays();

	return;
}

void YUV::initFromCfg(CfgYUVParams params) {

	m_bViewFileWritingEnabled = params.m_bViewFileWritingEnabled;
	m_bDepthFileWritingEnabled = params.m_bDepthFileWritingEnabled;

	m_sCurrentCameraName = params.m_sCurrentCameraName;
	m_asCameraNames = params.m_asCameraNames;

	m_sViewFilename = params.m_sViewFilename;
	m_sDepthFilename = params.m_sDepthFilename;

	m_currentCamParams = params.m_currentCamParams;
	m_aCamParams = params.m_aCamParams;

	m_acY = NULL; m_acU = NULL; m_acV = NULL; m_acD = NULL;
	m_asY = NULL; m_asU = NULL; m_asV = NULL; m_asD = NULL;

	m_uiWidth = params.m_uiWidth;
	m_uiHeight = params.m_uiHeight;

	m_uiFormat = params.m_uiFormat;

	m_dAovL = params.m_dAovL;
	m_dAovR = params.m_dAovR;
	m_dAovT = params.m_dAovT;
	m_dAovB = params.m_dAovB;

	m_dAovW = m_dAovR - m_dAovL;
	m_dAovH = m_dAovT - m_dAovB;

	m_uiPriority = params.m_uiPriority;

	m_uiViewBitsPerSample = params.m_uiViewBitsPerSample;
	m_uiViewBytesPerSample = (m_uiViewBitsPerSample <= 8) ? 1 : 2;
	//m_uiViewBytesPerSample = m_uiViewBitsPerSample >> 3;

	m_uiDepthBitsPerSample = params.m_uiDepthBitsPerSample;
	m_uiDepthBytesPerSample = (m_uiDepthBitsPerSample <= 8) ? 1 : 2;
	//m_uiDepthBytesPerSample = m_uiDepthBitsPerSample >> 3;
	
	m_uiViewChromaSubsampling = params.m_uiViewChromaSubsampling;
	m_uiDepthChromaSubsampling = params.m_uiDepthChromaSubsampling;

	if (m_uiViewChromaSubsampling == 400) {
		m_uiViewChromaWidth = 0;
		m_uiViewChromaHeight = 0;
	}
	else if (m_uiViewChromaSubsampling == 420) {
		m_uiViewChromaWidth = m_uiWidth >> 1;
		m_uiViewChromaHeight = m_uiHeight >> 1;
	}
	else if (m_uiViewChromaSubsampling == 444) {
		m_uiViewChromaWidth = m_uiWidth;
		m_uiViewChromaHeight = m_uiHeight;
	}

	if (m_uiDepthChromaSubsampling == 400) {
		m_uiDepthChromaWidth = 0;
		m_uiDepthChromaHeight = 0;
	}
	else if (m_uiDepthChromaSubsampling == 420) {
		m_uiDepthChromaWidth = m_uiWidth >> 1;
		m_uiDepthChromaHeight = m_uiHeight >> 1;
	}
	else if (m_uiDepthChromaSubsampling == 444) {
		m_uiDepthChromaWidth = m_uiWidth;
		m_uiDepthChromaHeight = m_uiHeight;
	}

	m_uiViewLumaFrameSizeInPixels = m_uiWidth * m_uiHeight;
	m_uiViewChromaFrameSizeInPixels = m_uiViewChromaWidth * m_uiViewChromaHeight;
	m_uiDepthLumaFrameSizeInPixels = m_uiWidth * m_uiHeight;
	m_uiDepthChromaFrameSizeInPixels = m_uiDepthChromaWidth * m_uiDepthChromaHeight;

	m_uiViewLumaFrameSizeInBytes = m_uiViewLumaFrameSizeInPixels * m_uiViewBytesPerSample;
	m_uiViewChromaFrameSizeInBytes = m_uiViewChromaFrameSizeInPixels * m_uiViewBytesPerSample;
	m_uiDepthLumaFrameSizeInBytes = m_uiDepthLumaFrameSizeInPixels * m_uiDepthBytesPerSample;
	m_uiDepthChromaFrameSizeInBytes = m_uiDepthChromaFrameSizeInPixels * m_uiDepthBytesPerSample;

	m_uiMaxDepthValue = pow(2, m_uiDepthBitsPerSample) - 1;
	m_uiMaxViewValue = pow(2, m_uiViewBitsPerSample) - 1;

	m_dZNear = params.m_dZNear;
	m_dZFar = params.m_dZFar;

	m_dInvMaxDepthValue = 1.0 / m_uiMaxDepthValue;
	m_dInvMaxViewValue = 1.0 / m_uiMaxViewValue;

	m_dInvZNear = 1.0 / m_dZNear;
	m_dInvZFar = 1.0 / m_dZFar;
	m_dInvZNearMinusInvZFar = m_dInvZNear - m_dInvZFar;
	m_dInvIZNMIZF = 1.0 / m_dInvZNearMinusInvZFar;

	initInOutArrays();
	initProcessingArrays();

	return;
}

YUV::YUV(CfgYUVParams params) {

	initFromCfg(params);
	return;
}

void YUV::initInOutArrays() {

	m_acY = new UChar[m_uiViewLumaFrameSizeInBytes];
	if (m_uiViewChromaSubsampling != 400) {
		m_acU = new UChar[m_uiViewChromaFrameSizeInBytes];
		m_acV = new UChar[m_uiViewChromaFrameSizeInBytes];
	}
	m_acD = new UChar[m_uiDepthLumaFrameSizeInBytes];

	m_asY = (UShort*)m_acY;
	m_asU = (UShort*)m_acU;
	m_asV = (UShort*)m_acV;
	m_asD = (UShort*)m_acD;

	return;
}

void YUV::initProcessingArrays() {

	m_aiYUVD = new Int*[4]; //Y,U,V,D
	for (int i = 0;i < 4;i++) m_aiYUVD[i] = new Int[m_uiViewLumaFrameSizeInPixels];

	m_afYUVD = (Float**)m_aiYUVD;

	return;
}

void YUV::cvtInputArraysToProcessing() {

	for (UInt c = 0;c < 4;c++) memset(m_afYUVD[c], 0, m_uiViewLumaFrameSizeInPixels * 4);

	if (m_uiDepthBitsPerSample <= 8) {
		for (UInt pp = 0;pp < m_uiViewLumaFrameSizeInPixels;pp++) {
			m_afYUVD[3][pp] = m_dInvMaxDepthValue * m_acD[pp] * m_dInvZNearMinusInvZFar + m_dInvZFar;
		}
	}//8bps
	else {
		for (UInt pp = 0;pp < m_uiViewLumaFrameSizeInPixels;pp++) {
			m_afYUVD[3][pp] = m_dInvMaxDepthValue * m_asD[pp] * m_dInvZNearMinusInvZFar + m_dInvZFar;
		}
	}//16bps

	if (m_uiViewBitsPerSample <= 8) {
		for (UInt h = 0, pp = 0;h < m_uiHeight;h++) {
			for (UInt w = 0;w < m_uiWidth;w++, pp++) {
				m_afYUVD[0][pp] = m_dInvMaxViewValue * m_acY[pp];
				if (m_uiViewChromaSubsampling == 444) {
					m_afYUVD[1][pp] = m_dInvMaxViewValue * m_acU[pp];
					m_afYUVD[2][pp] = m_dInvMaxViewValue * m_acV[pp];
				}//444
				else if (m_uiViewChromaSubsampling == 420) {
					UInt ppUV = (h >> 1)*(m_uiViewChromaWidth)+(w >> 1);
					m_afYUVD[1][pp] = m_dInvMaxViewValue * m_acU[ppUV];
					m_afYUVD[2][pp] = m_dInvMaxViewValue * m_acV[ppUV];
				}//420
			}//w
		}//h
	}//8bps
	else {
		for (UInt h = 0, pp = 0;h < m_uiHeight;h++) {
			for (UInt w = 0;w < m_uiWidth;w++, pp++) {
				m_afYUVD[0][pp] = m_dInvMaxViewValue * m_asY[pp];
				if (m_uiViewChromaSubsampling == 444) {
					m_afYUVD[1][pp] = m_dInvMaxViewValue * m_asU[pp];
					m_afYUVD[2][pp] = m_dInvMaxViewValue * m_asV[pp];
				}//444
				else if (m_uiViewChromaSubsampling == 420) {
					UInt ppUV = (h >> 1)*(m_uiViewChromaWidth)+(w >> 1);
					m_afYUVD[1][pp] = m_dInvMaxViewValue * m_asU[ppUV];
					m_afYUVD[2][pp] = m_dInvMaxViewValue * m_asV[ppUV];
				}//420
			}//w
		}//h
	}//16bps

	return;
}

void YUV::cvtProcessingArraysToOutput() {

	memset(m_acD, 0, m_uiDepthLumaFrameSizeInBytes);
	memset(m_acY, 0, m_uiViewLumaFrameSizeInBytes);
	if (m_uiViewChromaSubsampling != 400) {
		memset(m_acU, 0, m_uiViewChromaFrameSizeInBytes);
		memset(m_acV, 0, m_uiViewChromaFrameSizeInBytes);
	}

	if (m_bDepthFileWritingEnabled) {
		if (m_uiDepthBitsPerSample <= 8) {
			for (UInt pp = 0;pp < m_uiViewLumaFrameSizeInPixels;pp++) {
				m_acD[pp] = -(m_dInvZFar / m_afYUVD[3][pp] - 1) * (m_dInvIZNMIZF*m_afYUVD[3][pp])*m_uiMaxDepthValue;
			}
		}//8bps
		else {
			for (UInt pp = 0;pp < m_uiViewLumaFrameSizeInPixels;pp++) {
				m_asD[pp] = -(m_dInvZFar / m_afYUVD[3][pp] - 1) * (m_dInvIZNMIZF*m_afYUVD[3][pp])*m_uiMaxDepthValue;
			}
		}//16bps
	}
	if (m_bViewFileWritingEnabled) {
		if (m_uiViewBitsPerSample <= 8) {
			for (UInt h = 0, pp = 0;h < m_uiHeight;h++) {
				for (UInt w = 0;w < m_uiWidth;w++, pp++) {
					m_acY[pp] = m_uiMaxViewValue * m_afYUVD[0][pp];
					if (m_uiViewChromaSubsampling == 444) {
						m_acU[pp] = m_uiMaxViewValue * m_afYUVD[1][pp];
						m_acV[pp] = m_uiMaxViewValue * m_afYUVD[2][pp];
					}//444
					else if (m_uiViewChromaSubsampling == 420) {
						UInt ppUV = (h >> 1)*(m_uiViewChromaWidth)+(w >> 1);
						if (m_afYUVD[1][pp] != 0 && m_afYUVD[2][pp] != 0) {
							m_acU[ppUV] = m_uiMaxViewValue * m_afYUVD[1][pp];
							m_acV[ppUV] = m_uiMaxViewValue * m_afYUVD[2][pp];
						}
					}//420
				}//w
			}//h
		}//8bps
		else {
			for (UInt h = 0, pp = 0;h < m_uiHeight;h++) {
				for (UInt w = 0;w < m_uiWidth;w++, pp++) {
					m_asY[pp] = m_uiMaxViewValue * m_afYUVD[0][pp];
					if (m_uiViewChromaSubsampling == 444) {
						m_asU[pp] = m_uiMaxViewValue * m_afYUVD[1][pp];
						m_asV[pp] = m_uiMaxViewValue * m_afYUVD[2][pp];
					}//444
					else if (m_uiViewChromaSubsampling == 420) {
						UInt ppUV = (h >> 1)*(m_uiViewChromaWidth)+(w >> 1);
						if (m_afYUVD[1][pp] != 0 && m_afYUVD[2][pp] != 0) {
							m_asU[ppUV] = m_uiMaxViewValue * m_afYUVD[1][pp];
							m_asV[ppUV] = m_uiMaxViewValue * m_afYUVD[2][pp];
						}
					}//420
				}//w
			}//h
		}//16bps
	}

	return;
}

void YUV::readViewFrame(UInt frame) {

	FILE* fileYUV = NULL;
	fileYUV = fopen(m_sViewFilename.c_str(), "rb");

	Int64 offset = m_uiViewLumaFrameSizeInBytes + m_uiViewChromaFrameSizeInBytes + m_uiViewChromaFrameSizeInBytes;

  xfseek64(fileYUV, offset*frame, 0);
	fread(m_acY, 1, m_uiViewLumaFrameSizeInBytes, fileYUV);

	if (m_uiViewChromaSubsampling != 400) {
		fread(m_acU, 1, m_uiViewChromaFrameSizeInBytes, fileYUV);
		fread(m_acV, 1, m_uiViewChromaFrameSizeInBytes, fileYUV);
	}

	fclose(fileYUV);

	return;
}

void YUV::writeViewFrame(Bool append) {

	if (!m_bViewFileWritingEnabled) {
		return;
	}

	FILE* fileYUV = NULL;
	fileYUV = fopen(m_sViewFilename.c_str(), (append) ? "ab" : "wb");

	if (fileYUV == NULL) {
		fprintf(stdout, "Error: yuv file %s already opened\n", m_sViewFilename.c_str());
		return;
	}

	fwrite(m_acY, 1, m_uiViewLumaFrameSizeInBytes, fileYUV);

	if (m_uiViewChromaSubsampling != 400) {
		fwrite(m_acU, 1, m_uiViewChromaFrameSizeInBytes, fileYUV);
		fwrite(m_acV, 1, m_uiViewChromaFrameSizeInBytes, fileYUV);
	}

	fclose(fileYUV);

	return;
}

void YUV::readDepthFrame(UInt frame) {

	FILE* fileYUV = NULL;
	fileYUV = fopen(m_sDepthFilename.c_str(), "rb");

	Int64 offset = m_uiDepthLumaFrameSizeInBytes + m_uiDepthChromaFrameSizeInBytes + m_uiDepthChromaFrameSizeInBytes;

  xfseek64(fileYUV, offset*frame, 0);
	fread(m_acD, 1, m_uiDepthLumaFrameSizeInBytes, fileYUV);

	fclose(fileYUV);

	return;
}

void YUV::writeDepthFrame(Bool append) {

	if (!m_bDepthFileWritingEnabled) {
		return;
	}

	FILE* fileYUV = NULL;
	fileYUV = fopen(m_sDepthFilename.c_str(), (append) ? "ab" : "wb");

	if (fileYUV == NULL) {
		fprintf(stdout, "Error: yuv file %s already opened\n", m_sDepthFilename.c_str());
		return;
	}

	fwrite(m_acD, 1, m_uiDepthLumaFrameSizeInBytes, fileYUV);

	if (m_uiDepthChromaSubsampling != 400) {
		if (m_uiDepthBytesPerSample == 1) {
			UChar *tmp = new UChar[m_uiDepthChromaFrameSizeInPixels];
			for (UInt pp = 0;pp < m_uiDepthChromaFrameSizeInPixels;pp++) tmp[pp] = m_uiMaxDepthValue / 2;
			fwrite(tmp, 1, m_uiDepthChromaFrameSizeInBytes, fileYUV);
			fwrite(tmp, 1, m_uiDepthChromaFrameSizeInBytes, fileYUV);
			delete tmp;
		}
		else {
			UShort *tmp = new UShort[m_uiDepthChromaFrameSizeInPixels];
			for (UInt pp = 0;pp < m_uiDepthChromaFrameSizeInPixels;pp++) tmp[pp] = m_uiMaxDepthValue / 2;
			fwrite(tmp, 1, m_uiDepthChromaFrameSizeInBytes, fileYUV);
			fwrite(tmp, 1, m_uiDepthChromaFrameSizeInBytes, fileYUV);
			delete tmp;
		}
	}

	fclose(fileYUV);

	return;
}



