#include <iostream>
#include "avsSyntheser.h"


Int main(Int argc, char *argv[]) {

  if (argc < 2) {
    std::cout << "Use .cfg file\n";
    //system("PAUSE");
    return EXIT_FAILURE;
  }

  Config *cfg = new Config;
  //std::cout << "1\n";
  cfg->readCfgFile(argv[1]);

  //std::cout << "1\n";

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

  Syntheser *syn = new Syntheser(inputViews, outputViews, cfg);

  for (UInt frame = cfg->m_uiStartFrame;frame < cfg->m_uiNumberOfFrames + cfg->m_uiStartFrame;frame++) {
    std::cout << "FRAME " << frame << "\n";

    syn->loadInputViews(frame);
    syn->synthesize(frame);
    syn->saveOutputViews(frame != cfg->m_uiStartFrame);

  }

  system("PAUSE");

  return EXIT_SUCCESS;
}