#include <iostream>
#include "cYUV.h"
#include <vector>
#include <unordered_set>
#include <algorithm>

using namespace std;

int main()
{
#pragma region Params
	int _frames = 65;
	int _frameDensity = 1;
	string _depth = "";			// input depthMap file location
	string _depthOut = "";		// output depthMap file location
	string _cam = "";			// input camera yuv file location
	string _camOut = "";		// output camera yuv file location
	int _width = 1920;
	int _height = 1080;
	int bitesPerProbeCam = 10;
	int bitesPerProbeDepth = 10;
	int lumaChromaPropCam = 420;
	int lumaChromaPropDepth = 400;
#pragma endregion

#pragma region Variables
	cYUV<uint8_t> camOut(_width, _height, bitesPerProbeCam, lumaChromaPropCam);
	cYUV<uint8_t> cam(_width, _height, bitesPerProbeCam, lumaChromaPropCam);
	cYUV<UShort> depth(_width, _height, bitesPerProbeDepth, lumaChromaPropDepth);
	cYUV<UShort> depthOut(_width, _height, bitesPerProbeDepth, lumaChromaPropDepth);
	vector<float> sumY;
	vector<float> sumU;
	vector<float> sumV;
	vector<int> sumCount;
	vector<int> minD;
#pragma endregion

    for (int i = 0; i < (_height * _width); i++) {
		sumY.push_back(0);
		sumV.push_back(0);
		sumU.push_back(0);
		sumCount.push_back(0);
	}
	for (int frame = 0; frame < _frames; frame += _frameDensity) {
		depth.frameReader(_depth, frame);
		for (int h = 0; h < _height; h++) {
			for (int w = 0; w < _width; w++) {
				int luma = h * _width + w;
				if (frame == 0) {
					minD.push_back(depth.m_atY[luma]);
				}
				else {
					if (depth.m_atY[luma] < minD.at(luma)) {
						minD.at(luma) = depth.m_atY[luma];
					}
				}
			}
		}
	}
	for (int h = 0; h < _height; h++) {
		for (int w = 0; w < _width; w++) {
			int luma = h * _width + w;
			depthOut.m_atY[luma] = minD.at(luma);
		}
	}
	depthOut.frameWriter(_depthOut, 0);
	for (int frame = 0; frame < _frames; frame += _frameDensity) {
		cam.frameReader(_cam, frame);
		depth.frameReader(_depth, frame);
		for (int h = 0; h < _height; h++) {
			for (int w = 0; w < _width; w++) {
				int luma = h * _width + w;
				if (frame == 0) {
					vector<int> tmpY;
					outY.push_back(tmpY);
					if (h % 2 == 0 && w % 2 == 0) {
						vector<int> tmpU;
						vector<int> tmpV;
						outU.push_back(tmpU);
						outV.push_back(tmpV);
					}
				}
				if (depth.m_atY[luma] == minD.at(luma)) {
					sumY.at(luma) += (float)cam.m_atY[luma];
					if (h % 2 == 0 && w % 2 == 0) {
						int chroma = h / 2 * _width / 2 + w / 2;
						sumU.at(chroma) += (float)cam.m_atU[chroma];
						sumV.at(chroma) += (float)cam.m_atV[chroma];
					}
					sumCount.at(luma) += 1;
				}
			}
		}
	}
    for (int h = 0; h < _height; h++) {
        for (int w = 0; w < _width; w++) {
            int luma = h * _width + w;
            float averageLuma = sumY.at(luma) / (float)sumCount.at(luma);
            camOut.m_atY[luma] = averageLuma;
            if (h % 2 == 0 && w % 2 == 0) {
                int chroma = h / 2 * _width / 2 + w / 2;
                float averageU = sumU.at(chroma) / (float)sumCount.at(luma);
                float averageV = sumV.at(chroma) / (float)sumCount.at(luma);
                camOut.m_atU[chroma] = averageU;
                camOut.m_atV[chroma] = averageV;
            }
        }
    }
	camOut.frameWriter(_camOut, 0);
	return 0;
}

