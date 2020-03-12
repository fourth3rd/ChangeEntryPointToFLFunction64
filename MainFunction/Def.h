#pragma once

#include "stdafx.h"

#ifdef BASEDLL_EXPORTS
#define FLEXPORT __declspec(dllexport)
#else
#define FLEXPORT __declspec(dllimport)
#endif


#ifdef _DEBUG
#define LIB_IMPORT_PREFIX "../x64/Debug/"
#else
#define LIB_IMPORT_PREFIX "../x64/Release/"
#endif
