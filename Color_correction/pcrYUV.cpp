#include "pcrYUV.h"

YUV::~YUV() {

	if (m_acD) { delete m_acD; m_acD = NULL; m_asD = NULL; }
	if (m_acY) { delete m_acY; m_acY = NULL; m_asY = NULL; }
	if (m_acU) { delete m_acU; m_acU = NULL; m_asU = NULL; }
	if (m_acV) { delete m_acV; m_acV = NULL; m_asV = NULL; }

	return;

}

YUV::YUV() {

	m_bDepthFileWritingEnabled = false;
	m_bViewFileWritingEnabled = false;

	m_acD = NULL, m_asD = NULL;
	m_acY = NULL, m_asY = NULL;
	m_acU = NULL, m_asU = NULL;
	m_acV = NULL, m_asV = NULL;

	return;
}

YUV::YUV(UInt w, UInt h) {

	YUV();

	m_iWidth = w;
	m_iHeight = h;
	m_uiDepthLumaFrameSizeInPixels = m_iWidth * m_iHeight;
	m_uiViewLumaFrameSizeInPixels = m_iWidth * m_iHeight;

	initProcessingArrays();

	return;
}

void YUV::enableProcessingArraysOutputting(String outputViewName, Double znear, Double zfar) {

	m_bViewFileWritingEnabled = true;

	m_dZNear = znear;
	m_dZFar = zfar;

	m_uiDepthChromaSubsampling = 420;
	m_uiDepthBitsPerSample = 10;
	m_uiDepthBytesPerSample = 2;

	m_uiViewChromaSubsampling = 420;
	m_uiViewBitsPerSample = 10;
	m_uiViewBytesPerSample = 2;

	m_uiMaxDepthValue = (1 << (m_uiDepthBitsPerSample)) - 1;
	m_dInvMaxDepthValue = 1.0 / m_uiMaxDepthValue;

	m_uiMaxViewValue = (1 << (m_uiViewBitsPerSample)) - 1;
	m_dInvMaxViewValue = 1.0 / m_uiMaxViewValue;

	m_dInvZNear = 1.0 / m_dZNear;
	m_dInvZFar = 1.0 / m_dZFar;
	m_dInvZNearMinusInvZFar = m_dInvZNear - m_dInvZFar;
	m_dInvIZNMIZF = 1.0 / m_dInvZNearMinusInvZFar;

	m_sViewFilename = outputViewName;

	if (m_uiDepthChromaSubsampling == 400) {
		m_uiDepthChromaWidth = 0;
		m_uiDepthChromaHeight = 0;
	}		
	else if (m_uiDepthChromaSubsampling == 420) {
		m_uiDepthChromaWidth = m_iWidth / 2;
		m_uiDepthChromaHeight = m_iHeight / 2;
	}		
	else if (m_uiDepthChromaSubsampling == 444) {
		m_uiDepthChromaWidth = m_iWidth;
		m_uiDepthChromaHeight = m_iHeight;
	}

	if (m_uiViewChromaSubsampling == 400) {
		m_uiViewChromaWidth = 0;
		m_uiViewChromaHeight = 0;
	}
	else if (m_uiViewChromaSubsampling == 420) {
		m_uiViewChromaWidth = m_iWidth / 2;
		m_uiViewChromaHeight = m_iHeight / 2;
	}
	else if (m_uiViewChromaSubsampling == 444) {
		m_uiViewChromaWidth = m_iWidth;
		m_uiViewChromaHeight = m_iHeight;
	}

	m_uiDepthChromaFrameSizeInPixels = m_uiDepthChromaWidth * m_uiDepthChromaHeight;
	m_uiDepthLumaFrameSizeInBytes = m_uiDepthLumaFrameSizeInPixels * m_uiDepthBytesPerSample;
	m_uiDepthChromaFrameSizeInBytes = m_uiDepthChromaFrameSizeInPixels * m_uiDepthBytesPerSample;

	m_uiViewChromaFrameSizeInPixels = m_uiViewChromaWidth * m_uiViewChromaHeight;
	m_uiViewLumaFrameSizeInBytes = m_uiViewLumaFrameSizeInPixels * m_uiViewBytesPerSample;
	m_uiViewChromaFrameSizeInBytes = m_uiViewChromaFrameSizeInPixels * m_uiViewBytesPerSample;

	initInOutArrays();

	return;
}

void YUV::initFromCfg(CfgYUVParams params) {

	m_bDepthFileWritingEnabled = params.m_bDepthFileWritingEnabled;
	m_bViewFileWritingEnabled = params.m_bViewFileWritingEnabled;

	m_sCurrentCameraName = params.m_sCurrentCameraName;
	m_asCameraNames = params.m_asCameraNames;

	m_sDepthFilename = params.m_sDepthFilename;
	m_sViewFilename = params.m_sViewFilename;

	m_currentCamParams = params.m_currentCamParams;
	m_aCamParams = params.m_aCamParams;

	m_acD = NULL, m_acY = NULL, m_acU = NULL, m_acV = NULL;
	m_asD = NULL, m_asY = NULL, m_asU = NULL, m_asV = NULL;

	m_iWidth = params.m_iWidth;
	m_iHeight = params.m_iHeight;

	m_uiFormat = params.m_uiFormat;

	m_dAovL = params.m_dAovL;
	m_dAovR = params.m_dAovR;
	m_dAovT = params.m_dAovT;
	m_dAovB = params.m_dAovB;

	m_dAovW = m_dAovR - m_dAovL;
	m_dAovH = m_dAovT - m_dAovB;
	
	m_uiDepthBitsPerSample = params.m_uiDepthBitsPerSample;
	m_uiDepthBytesPerSample = (m_uiDepthBitsPerSample <= 8) ? 1 : 2;

	m_uiViewBitsPerSample = params.m_uiViewBitsPerSample;
	m_uiViewBytesPerSample = (m_uiViewBitsPerSample <= 8) ? 1 : 2;

	m_uiDepthChromaSubsampling = params.m_uiDepthChromaSubsampling;
	m_uiViewChromaSubsampling = params.m_uiViewChromaSubsampling;

	if (m_uiDepthChromaSubsampling == 400) {
		m_uiDepthChromaWidth = 0;
		m_uiDepthChromaHeight = 0;
	}
	else if (m_uiDepthChromaSubsampling == 420) {
		m_uiDepthChromaWidth = m_iWidth >> 1;
		m_uiDepthChromaHeight = m_iHeight >> 1;
	}
	else if (m_uiDepthChromaSubsampling == 444) {
		m_uiDepthChromaWidth = m_iWidth;
		m_uiDepthChromaHeight = m_iHeight;
	}

	if (m_uiViewChromaSubsampling == 400) {
		m_uiViewChromaWidth = 0;
		m_uiViewChromaHeight = 0;
	}
	else if (m_uiViewChromaSubsampling == 420) {
		m_uiViewChromaWidth = m_iWidth >> 1;
		m_uiViewChromaHeight = m_iHeight >> 1;
	}
	else if (m_uiViewChromaSubsampling == 444) {
		m_uiViewChromaWidth = m_iWidth;
		m_uiViewChromaHeight = m_iHeight;
	}

	m_uiDepthLumaFrameSizeInPixels = m_iWidth * m_iHeight;
	m_uiDepthChromaFrameSizeInPixels = m_uiDepthChromaWidth * m_uiDepthChromaHeight;

	m_uiDepthLumaFrameSizeInBytes = m_uiDepthLumaFrameSizeInPixels * m_uiDepthBytesPerSample;
	m_uiDepthChromaFrameSizeInBytes = m_uiDepthChromaFrameSizeInPixels * m_uiDepthBytesPerSample;

	m_uiViewLumaFrameSizeInPixels = m_iWidth * m_iHeight;
	m_uiViewChromaFrameSizeInPixels = m_uiViewChromaWidth * m_uiViewChromaHeight;

	m_uiViewLumaFrameSizeInBytes = m_uiViewLumaFrameSizeInPixels * m_uiViewBytesPerSample;
	m_uiViewChromaFrameSizeInBytes = m_uiViewChromaFrameSizeInPixels * m_uiViewBytesPerSample;
	
	m_uiMaxDepthValue = (1 << m_uiDepthBitsPerSample) - 1;
	m_uiMaxViewValue = (1 << m_uiViewBitsPerSample) - 1;

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

	m_acD = new UChar[m_uiDepthLumaFrameSizeInBytes]; m_asD = (UShort*)m_acD;
	m_acY = new UChar[m_uiViewLumaFrameSizeInBytes]; m_asY = (UShort*)m_acY;
	m_acU = new UChar[m_uiViewChromaFrameSizeInBytes]; m_asU = (UShort*)m_acU;
	m_acV = new UChar[m_uiViewChromaFrameSizeInBytes]; m_asV = (UShort*)m_acV;

	return;
}

void YUV::initProcessingArrays() {

	m_aiYUVD = new Int*[4];
	for (Int c = 0;c < 4;c++) m_aiYUVD[c] = new Int[m_uiDepthLumaFrameSizeInPixels];
	m_afYUVD = (Float**)m_aiYUVD;

	return;
}

void YUV::cvtInputArraysToProcessing() {
	
	for (Int c = 0;c < 4;c++) memset(m_afYUVD[c], 0, m_uiDepthLumaFrameSizeInPixels * 4);

	if (m_uiDepthBitsPerSample <= 8) {
		for (UInt pp = 0;pp < m_uiDepthLumaFrameSizeInPixels;pp++) {
			m_afYUVD[3][pp] = m_dInvMaxDepthValue * m_acD[pp] * m_dInvZNearMinusInvZFar + m_dInvZFar;
		}
	}//8bps
	else {
		for (UInt pp = 0;pp < m_uiDepthLumaFrameSizeInPixels;pp++) {
			m_afYUVD[3][pp] = m_dInvMaxDepthValue * m_asD[pp] * m_dInvZNearMinusInvZFar + m_dInvZFar;
		}
	}//16bps

	if (m_uiViewBitsPerSample <= 8) {
		for (Int h = 0, pp = 0;h < m_iHeight;h++) {
			for (Int w = 0;w < m_iWidth;w++, pp++) {
				m_afYUVD[0][pp] = m_dInvMaxViewValue * m_acY[pp];
				if (m_uiViewChromaSubsampling == 444) {
					m_afYUVD[1][pp] = m_dInvMaxViewValue * m_acU[pp];
					m_afYUVD[2][pp] = m_dInvMaxViewValue * m_acV[pp];
				}//444
				else if (m_uiViewChromaSubsampling == 420) {
					Int ppUV = (h >> 1)*(m_uiViewChromaWidth)+(w >> 1);
					m_afYUVD[1][pp] = m_dInvMaxViewValue * m_acU[ppUV];
					m_afYUVD[2][pp] = m_dInvMaxViewValue * m_acV[ppUV];
				}//420
			}//w
		}//h
	}//8bps
	else {
		for (Int h = 0, pp = 0;h < m_iHeight;h++) {
			for (Int w = 0;w < m_iWidth;w++, pp++) {
				m_afYUVD[0][pp] = m_dInvMaxViewValue * m_asY[pp];
				if (m_uiViewChromaSubsampling == 444) {
					m_afYUVD[1][pp] = m_dInvMaxViewValue * m_asU[pp];
					m_afYUVD[2][pp] = m_dInvMaxViewValue * m_asV[pp];
				}//444
				else if (m_uiViewChromaSubsampling == 420) {
					Int ppUV = (h >> 1)*(m_uiViewChromaWidth)+(w >> 1);
					m_afYUVD[1][pp] = m_dInvMaxViewValue * m_asU[ppUV];
					m_afYUVD[2][pp] = m_dInvMaxViewValue * m_asV[ppUV];
				}//420
			}//w
		}//h
	}//16bps

#if RGB_PROCESSING
	Double R, G, B, Y, U, V;
	for (UInt pp = 0;pp < m_uiViewLumaFrameSizeInPixels;pp++) {
		Y = m_afYUVD[0][pp];
		U = m_afYUVD[1][pp] - 0.5;
		V = m_afYUVD[2][pp] - 0.5;
		R = Clip3(0.0, Y + 1.402*V, 1.0);
		G = Clip3(0.0, Y - 0.344*U - 0.714*V, 1.0);
		B = Clip3(0.0, Y + 1.772*U, 1.0);
		m_afYUVD[0][pp] = R;
		m_afYUVD[1][pp] = G;
		m_afYUVD[2][pp] = B;
	}
#endif

	return;
}

void YUV::cvtProcessingArraysToOutput() {

#if RGB_PROCESSING
	Double R, G, B, Y, U, V;
	for (UInt pp = 0;pp < m_uiViewLumaFrameSizeInPixels;pp++) {
		R = m_afYUVD[0][pp];
		G = m_afYUVD[1][pp];
		B = m_afYUVD[2][pp];
		Y = Clip3(0.0, 0.299*R + 0.587*G + 0.114*B, 1.0);
		U = Clip3(0.0, -0.169*R - 0.331*G + 0.499*B + 0.5, 1.0);
		V = Clip3(0.0, 0.499*R - 0.418*G - 0.0813*B + 0.5, 1.0);
		m_afYUVD[0][pp] = Y;
		m_afYUVD[1][pp] = U;
		m_afYUVD[2][pp] = V;
	}
#endif
	
	memset(m_acD, 0, m_uiDepthLumaFrameSizeInBytes);
	memset(m_acY, 0, m_uiViewLumaFrameSizeInBytes);
	memset(m_acU, 0, m_uiViewChromaFrameSizeInBytes);
	memset(m_acV, 0, m_uiViewChromaFrameSizeInBytes);
	
	if (m_bDepthFileWritingEnabled) {
		if (m_uiDepthBitsPerSample <= 8) {
			for (UInt pp = 0;pp < m_uiDepthLumaFrameSizeInPixels;pp++) {
				m_acD[pp] = -(m_dInvZFar / m_afYUVD[3][pp] - 1) * (m_dInvIZNMIZF*m_afYUVD[3][pp])*m_uiMaxDepthValue;
			}
		}//8bps
		else {
			for (UInt pp = 0;pp < m_uiDepthLumaFrameSizeInPixels;pp++) {
				m_asD[pp] = -(m_dInvZFar / m_afYUVD[3][pp] - 1) * (m_dInvIZNMIZF*m_afYUVD[3][pp])*m_uiMaxDepthValue;
			}
		}//16bps
	}

	for (Int h = 0, pp = 0;h < m_iHeight;h++) {
		for (Int w = 0;w < m_iWidth;w++, pp++) {
			for (Int c = 0;c < 4;c++) {
				m_afYUVD[c][pp] = Clip3(0.0, m_afYUVD[c][pp], 1.0);
			}
		}
	}

	if (m_bViewFileWritingEnabled) {
		if (m_uiViewBitsPerSample <= 8) {
			for (Int h = 0, pp = 0;h < m_iHeight;h++) {
				for (Int w = 0;w < m_iWidth;w++, pp++) {
					m_acY[pp] = m_uiMaxViewValue * m_afYUVD[0][pp];
					if (m_uiViewChromaSubsampling == 444) {
						m_acU[pp] = m_uiMaxViewValue * m_afYUVD[1][pp];
						m_acV[pp] = m_uiMaxViewValue * m_afYUVD[2][pp];
					}//444
					else if (m_uiViewChromaSubsampling == 420) {
						Int ppUV = (h >> 1)*(m_uiViewChromaWidth)+(w >> 1);
						if (m_afYUVD[1][pp] != 0 && m_afYUVD[2][pp] != 0) {
							m_acU[ppUV] = m_uiMaxViewValue * m_afYUVD[1][pp];
							m_acV[ppUV] = m_uiMaxViewValue * m_afYUVD[2][pp];
						}
					}//420
				}//w
			}//h
		}//8bps
		else {
			for (Int h = 0, pp = 0;h < m_iHeight;h++) {
				for (Int w = 0;w < m_iWidth;w++, pp++) {
					m_asY[pp] = m_uiMaxViewValue * m_afYUVD[0][pp];
					if (m_uiViewChromaSubsampling == 444) {
						m_asU[pp] = m_uiMaxViewValue * m_afYUVD[1][pp];
						m_asV[pp] = m_uiMaxViewValue * m_afYUVD[2][pp];
					}//444
					else if (m_uiViewChromaSubsampling == 420) {
						Int ppUV = (h >> 1)*(m_uiViewChromaWidth)+(w >> 1);
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

void YUV::readDepthFrame(UInt frame) {

	FILE* fileYUV = NULL;
	fileYUV = fopen(m_sDepthFilename.c_str(), "rb");

	Int64 offset = m_uiDepthLumaFrameSizeInBytes + m_uiDepthChromaFrameSizeInBytes + m_uiDepthChromaFrameSizeInBytes;

  xfseek64(fileYUV, offset*frame, 0);
	fread(m_acD, 1, m_uiDepthLumaFrameSizeInBytes, fileYUV);

	fclose(fileYUV);

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


