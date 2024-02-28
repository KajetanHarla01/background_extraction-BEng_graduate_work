//  Original authors:
//
//  Adrian Dziembowski,  adrian.dziembowski@put.poznan.pl
//  Dawid Mieloch,       dawid.mieloch@put.poznan.pl
//  S³awomir Ró¿ek
//
//  Poznañ University of Technology, Poznañ, Poland

#include <iostream>
#include "pcrParams.h"
#include "pcrYUV.h"
#include "pcrConfig.h"
#include "pcrRefiner.h"

int main(Int argc, Char *argv[]) {

	if (argc < 2) {
		std::cout << "Use .cfg file\n";
#if _MSC_VER
		system("PAUSE");
#endif
		return EXIT_FAILURE;
	}

	Config *cfg = new Config;
	cfg->readCfgFile(argv[1]);

	cArray<YUV*> inputViews;
	for (UInt i = 0;i < cfg->m_uiNumberOfInputViews;i++) {
		YUV *view = new YUV(cfg->m_cfgInputs[i]);
		inputViews.push_back(view);
	}

	cArray<YUV*> outputViews;
	for (UInt o = 0;o < cfg->m_uiNumberOfOutputViews;o++) {
		YUV* view = new YUV(cfg->m_cfgOutputs[o]);
		outputViews.push_back(view);
	}

	Refiner *ref = new Refiner(inputViews, outputViews, cfg);

	std::cout << "\nBACKGROUND CALCULATION\n";
	ref->initBackgroundVector(cfg->m_uiNumberOfFrames);
	for (UInt frame = cfg->m_uiStartFrame;frame < cfg->m_uiNumberOfFrames + cfg->m_uiStartFrame;frame += BACKGROUND_T_STEP) {
		std::cout << "FRAME " << frame << "\n";
		ref->loadInputViews(frame);
		ref->loadInputDepthMaps(frame);
		ref->clearProcessingArrays();
		ref->cvtInputArraysToProcessing();
		ref->fillBackgroundVector((frame - cfg->m_uiStartFrame) / BACKGROUND_T_STEP);
	}
	ref->calcBackground();
	ref->deinitBackgroundVector();

	for (UInt frame = cfg->m_uiStartFrame;frame < cfg->m_uiNumberOfFrames + cfg->m_uiStartFrame;frame++) {
		std::cout << "\nFRAME " << frame << "\n";

		ref->loadInputDepthMaps(frame);
		ref->loadInputViews(frame);
		ref->clearProcessingArrays();
		ref->cvtInputArraysToProcessing();

		ref->refine(frame); // perform color refinement

		ref->saveOutputViews(frame != cfg->m_uiStartFrame);
	}

	return EXIT_SUCCESS;
}