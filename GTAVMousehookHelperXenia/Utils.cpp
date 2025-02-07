#include "stdafx.h"
#include "Utils.h"

namespace Utils{
bool CompareData(const BYTE* pData, const BYTE* bMask, size_t patternSize) {
    for (size_t i = 0; i < patternSize; i++) {
        if (bMask[i] != '\x00' && pData[i] != bMask[i])
            return false;
    }
    return true;
}


uintptr_t PatternScan(const char* patternStr) {
    size_t patternSize = strlen(patternStr);

    for (uintptr_t addr = XBOX_MEMORY_START; addr < XBOX_MEMORY_END; addr += 4) {
        BYTE* memAddr = reinterpret_cast<BYTE*>(addr);

        if (MmIsAddressValid(memAddr) && CompareData(memAddr, (BYTE*)patternStr, patternSize)) {
            return addr;
        }
    }
    return 0;
}
}