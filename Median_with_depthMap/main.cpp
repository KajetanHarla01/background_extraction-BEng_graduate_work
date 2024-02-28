#include <iostream>
#include <fstream>
#include "cYUV.h"
#include <vector>
#include <algorithm>
#include <unordered_set>
using namespace std;

int main()
{

#pragma region Params
	int _frames = 1500;
	int _frameDensity = 1;
	int _minPixelsCount = 100;   // maximum_value = _frames / _frameDensity
	int _startLatitude = 200;
	int _deltaLatitude = 200;
	int _maxLatitude = 1000;
	string _depth = "";			// input depthMap file location
	string _depthOut = "";		// output depthMap file location
	string _cam = "";			// input camera yuv file location
	string _camOut = "";		// output camera yuv file location
	int _width = 1920;
	int _height = 1080;
	int bitesPerProbeCam = 8;
	int bitesPerProbeDepth = 16;
	int lumaChromaPropCam = 420;
	int lumaChromaPropDepth = 400;
#pragma endregion

#pragma region Variables
	cYUV<uint8_t> camOut(_width, _height, bitesPerProbeCam, lumaChromaPropCam);
	cYUV<uint8_t> cam(_width, _height, bitesPerProbeCam, lumaChromaPropCam);
	cYUV<UShort> depth(_width, _height, bitesPerProbeDepth, lumaChromaPropDepth);
	cYUV<UShort> depthOut(_width, _height, bitesPerProbeDepth, lumaChromaPropDepth);
	vector<vector<int>> outY;
	vector<vector<int>> outU;
	vector<vector<int>> outV;
	unordered_set<int> correctPixels;
	vector<int> minD;
	bool repeatPixelsFill = true;
	bool getAllPixels = false;
#pragma endregion

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
	while (repeatPixelsFill) {
		repeatPixelsFill = false;
		for (int frame = 0; frame < _frames; frame += _frameDensity) {
			cam.frameReader(_cam, frame);
			depth.frameReader(_depth, frame);
			for (int h = 0; h < _height; h++) {
				for (int w = 0; w < _width; w++) {
					int luma = h * _width + w;
					if (correctPixels.find(luma) == correctPixels.end()) {
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
						if (getAllPixels || depth.m_atY[luma] <= _startLatitude + minD.at(luma)) {
							outY.at(luma).push_back(cam.m_atY[luma]);
							if (h % 2 == 0 && w % 2 == 0) {
								int chroma = h / 2 * _width / 2 + w / 2;
								outU.at(chroma).push_back(cam.m_atU[chroma]);
								outV.at(chroma).push_back(cam.m_atV[chroma]);
							}
						}
					}
				}
			}
		}
        for (int h = 0; h < _height; h++) {
            for (int w = 0; w < _width; w++) {
                int luma = h * _width + w;
                if (correctPixels.find(luma) == correctPixels.end()) {
                    if (outY.at(luma).size() >= _minPixelsCount) {
                        correctPixels.insert(luma);
                    }
                    else {
                        repeatPixelsFill = true;
                        outY.at(luma).clear();
                        if (h % 2 == 0 && w % 2 == 0) {
                            int chroma = h / 2 * _width / 2 + w / 2;
                            outU.at(chroma).clear();
                            outV.at(chroma).clear();
                        }
                    }
                }
            }
        }
        cout << "Latitude: " << _startLatitude << " CorrectPixels: " << correctPixels.size() << endl;
        _startLatitude += _deltaLatitude;
        if (_startLatitude >= _maxLatitude) {
            getAllPixels = true;
        }
	}
	for (int h = 0; h < _height; h++) {
		for (int w = 0; w < _width; w++) {
			int luma = h * _width + w;
			sort(outY.at(luma).begin(), outY.at(luma).end());
			int medianY = outY.at(luma).size() % 2 == 1 ? outY.at(luma).at((int)outY.at(luma).size() / 2) : (outY.at(luma).at((int)outY.at(luma).size() / 2) + outY.at(luma).at((int)(outY.at(luma).size() - 1) / 2)) / 2;
			camOut.m_atY[luma] = medianY;
			if (h % 2 == 0 && w % 2 == 0) {
				int chroma = h / 2 * _width / 2 + w / 2;
				sort(outU.at(chroma).begin(), outU.at(chroma).end());
				sort(outV.at(chroma).begin(), outV.at(chroma).end());
				int medianU = outU.at(chroma).size() % 2 == 1 ? outU.at(chroma).at((int)outU.at(chroma).size() / 2) : (outU.at(chroma).at((int)outU.at(chroma).size() / 2) + outU.at(chroma).at((int)(outU.at(chroma).size() - 1) / 2)) / 2;
				int medianV = outV.at(chroma).size() % 2 == 1 ? outV.at(chroma).at((int)outV.at(chroma).size() / 2) : (outV.at(chroma).at((int)outV.at(chroma).size() / 2) + outV.at(chroma).at((int)(outV.at(chroma).size() - 1) / 2)) / 2;
				camOut.m_atU[chroma] = medianU;
				camOut.m_atV[chroma] = medianV;
			}
		}
	}
	camOut.frameWriter(_camOut, 0);
	return 0;
}
