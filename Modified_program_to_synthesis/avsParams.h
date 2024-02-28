#ifndef __AVS_PARAMS_H__
#define __AVS_PARAMS_H__

#define OUTPUT_INTERMEDIATES	0
#define ENABLE_INPAINTING		1

#define INTERPOLATE_COLOR_USING_4_PIXELS	1 //1
#define AVERAGED_INPAINTING					3 //3
#define INP2_SIZE							5 //5

#define _CRT_SECURE_NO_WARNINGS

#ifdef _MSC_VER
#define xfseek64 _fseeki64
#else
#define _FILE_OFFSET_BITS 64
#define xfseek64 fseeko64
#endif

#endif
