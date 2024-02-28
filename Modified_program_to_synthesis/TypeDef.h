#ifndef _TYPEDEF__
#define _TYPEDEF__

#include "avsParams.h"

// ====================================================================================================================
// Basic type redefinition
// ====================================================================================================================

typedef       void                Void;
typedef       bool                Bool;

typedef       char                Char;
typedef       unsigned char       UChar;
typedef       short               Short;
typedef       unsigned short      UShort;
typedef       int                 Int;
typedef       unsigned int        UInt;
typedef       double              Double;
typedef		  float				  Float;

// ====================================================================================================================
// String type redefinition
// ====================================================================================================================
#include <string>

typedef std::string String;

// ====================================================================================================================
// 64-bit integer type
// ====================================================================================================================

#ifdef _MSC_VER
typedef       __int64             Int64;

#if _MSC_VER <= 1200 // MS VC6
typedef       __int64             UInt64;   // MS VC6 does not support unsigned __int64 to Double conversion
#else
typedef       unsigned __int64    UInt64;
#endif

#else

typedef       long long           Int64;
typedef       unsigned long long  UInt64;

#endif

// ====================================================================================================================
// Type definition
// ====================================================================================================================

//typedef       UChar           Pxl;                       ///< 8-bit pixel type
//typedef       Short           Pel;                       ///< 16-bit pixel type
//typedef       Int             TCoeff;                    ///< transform coefficient
//typedef       Double          MatrixComputationalType;   ///Type used for matrix algebra

#include <vector>

template <typename t> class cArray : public std::vector<t> {};

#define Max(x, y)                   ((x)>(y)?(x):(y))
#define Min(x, y)                   ((x)<(y)?(x):(y))   

#define Max3(x,y,z)					(Max(Max(x,y),z))
#define Min3(x,y,z)					(Min(Min(x,y),z))

#define PI							3.141592654
#define PI2							6.283185307
#define PI_HALF						1.570796327
#define INV_PI						0.318309886
#define INV_PI2						0.159154943

#define BIG_FLOAT_VALUE				1000000.0

#endif


