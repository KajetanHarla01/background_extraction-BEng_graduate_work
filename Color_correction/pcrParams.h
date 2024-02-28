#ifndef __PCR_PARAMS_H__
#define __PCR_PARAMS_H__

#define ESTIMATE_TIME			0
#define RGB_PROCESSING			0

#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#define xfseek64 _fseeki64
#else
#define _FILE_OFFSET_BITS 64
#define xfseek64 fseeko64
#endif

#endif
