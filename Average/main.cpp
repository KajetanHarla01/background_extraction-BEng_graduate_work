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
	float currentLatitude = 0.0f;
	float startLatitude = 30.0f;
	float latitudeInc = 5.0f;
	int minPixelCount = 10;
	string _cam = "";			// input camera yuv file location
	string _camOut = "";		// output camera yuv file location
	int _width = 1920;
	int _height = 1080;
	int bitesPerProbeCam = 8;
	int lumaChromaPropCam = 420;
#pragma endregion

#pragma region Variables
	cYUV<uint8_t> camOut(_width, _height, bitesPerProbeCam, lumaChromaPropCam);
	cYUV<uint8_t> cam(_width, _height, bitesPerProbeCam, lumaChromaPropCam);
	vector<float> sumLuma;
	vector<float> sumChromaU;
	vector<float> sumChromaV;
	vector<float> sumLumaLat;
	vector<float> sumChromaULat;
	vector<float> sumChromaVLat;
	vector<int> sumLumaLatCount;
	vector<int> sumChromaULatCount;
	vector<int> sumChromaVLatCount;
	unordered_set<int> correctPixelsLuma;
	unordered_set<int> correctPixelsChromaU;
	unordered_set<int> correctPixelsChromaV;
	bool repeatPixelsFill = true;
	bool getAllPixels = false;
#pragma endregion
	currentLatitude = startLatitude;
	for (int i = 0; i < (_height * _width); i++) {
		sumLuma.push_back(0);
		sumChromaU.push_back(0);
		sumChromaV.push_back(0);
		sumLumaLat.push_back(0);
		sumChromaULat.push_back(0);
		sumChromaVLat.push_back(0);
		sumLumaLatCount.push_back(0);
		sumChromaULatCount.push_back(0);
		sumChromaVLatCount.push_back(0);
	}
	for (int frame = 0; frame < _frames; frame += _frameDensity) {
		cam.frameReader(_cam, frame);
		for (int h = 0; h < _height; h++) {
			for (int w = 0; w < _width; w++) {
				int luma = h * _width + w;
				sumLuma.at(luma) += (float)cam.m_atY[luma];
				if (h % 2 == 0 && w % 2 == 0) {
					int chroma = h / 2 * _width / 2 + w / 2;
					sumChromaU.at(chroma) += (float)cam.m_atU[chroma];
					sumChromaV.at(chroma) += (float)cam.m_atV[chroma];
				}
			}
		}
	}
	while (repeatPixelsFill) {
		repeatPixelsFill = false;
		for (int frame = 0; frame < _frames; frame += _frameDensity) {
			cam.frameReader(_cam, frame);
			for (int h = 0; h < _height; h++) {
				for (int w = 0; w < _width; w++) {
					int luma = h * _width + w;
					float averageLuma = sumLuma.at(luma) / (float)(_frames / _frameDensity);
					if (correctPixelsLuma.find(luma) == correctPixelsLuma.end() && ((float)cam.m_atY[luma] > averageLuma - currentLatitude && (float)cam.m_atY[luma] < averageLuma + currentLatitude)) {
						sumLumaLat.at(luma) += (float)cam.m_atY[luma];
						sumLumaLatCount[luma]++;
					}
					if (h % 2 == 0 && w % 2 == 0) {
						int chroma = h / 2 * _width / 2 + w / 2;
						float averageChromaU = sumChromaU.at(chroma) / (float)(_frames / _frameDensity);
						float averageChromaV = sumChromaV.at(chroma) / (float)(_frames / _frameDensity);
						if (correctPixelsChromaU.find(chroma) == correctPixelsChromaU.end() && ((float)cam.m_atU[chroma] > averageChromaU - currentLatitude && (float)cam.m_atU[chroma] < averageChromaU + currentLatitude)) {
							sumChromaULat.at(chroma) += (float)cam.m_atU[chroma];
							sumChromaULatCount[chroma]++;
						}
						if (correctPixelsChromaV.find(chroma) == correctPixelsChromaV.end() && ((float)cam.m_atV[chroma] > averageChromaV - currentLatitude && (float)cam.m_atV[chroma] < averageChromaV + currentLatitude)) {
							sumChromaVLat.at(chroma) += (float)cam.m_atV[chroma];
							sumChromaVLatCount[chroma]++;
						}
					}
				}
			}
		}
		int min = *min_element(sumLumaLatCount.begin(), sumLumaLatCount.end());
		cout << "Average latitude: " << currentLatitude << " Min luma count: " << min << endl;
		for (int h = 0; h < _height; h++) {
			for (int w = 0; w < _width; w++) {
				int luma = h * _width + w;
				if (sumLumaLatCount.at(luma) < minPixelCount) {
					repeatPixelsFill = true;
					sumLumaLat.at(luma) = 0;
					sumLumaLatCount.at(luma) = 0;
				}
				else if (correctPixelsLuma.find(luma) == correctPixelsLuma.end()){
					camOut.m_atY[luma] = (int)sumLumaLat.at(luma) / sumLumaLatCount.at(luma);
					correctPixelsLuma.insert(luma);
				}
				if (h % 2 == 0 && w % 2 == 0) {
					int chroma = h / 2 * _width / 2 + w / 2;
					if (sumChromaULatCount.at(chroma) < minPixelCount) {
						repeatPixelsFill = true;
						sumChromaULat.at(chroma) = 0;
						sumChromaULatCount.at(chroma) = 0;
					}
					else if (correctPixelsChromaU.find(chroma) == correctPixelsChromaU.end()) {
						camOut.m_atU[chroma] = (int)sumChromaULat.at(chroma) / sumChromaULatCount.at(chroma);
						correctPixelsChromaU.insert(chroma);
					}
					if (sumChromaVLatCount.at(chroma) < minPixelCount) {
						repeatPixelsFill = true;
						sumChromaVLat.at(chroma) = 0;
						sumChromaVLatCount.at(chroma) = 0;
					}
					else if (correctPixelsChromaV.find(chroma) == correctPixelsChromaV.end()) {
						camOut.m_atV[chroma] = (int)sumChromaVLat.at(chroma) / sumChromaVLatCount.at(chroma);
						correctPixelsChromaV.insert(chroma);
					}
				}
			}
		}
		currentLatitude += latitudeInc;
	}
	out.frameWriter(_camOut, 0);
}
