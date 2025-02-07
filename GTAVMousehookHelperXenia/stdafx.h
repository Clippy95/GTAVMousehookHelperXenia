// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once
#include "Detour.h"
#include <xtl.h>
#include <xboxmath.h>
#include <xkelib.h>
#include <xbdm.h>
#include <vector>
#include <string>
#include <cstdint>
#include "Utils.h"
using namespace std;

struct FunctionsAddr {
	uint32_t MAYBEComputeOrbitOrientation_addr;
	uint32_t write_delta_detour;
};
// TODO: reference additional headers your program requires here
