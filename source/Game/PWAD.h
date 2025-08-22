#pragma once
#include <cstdint>
#include <string>

class WadFile;

namespace PWAD {
    void initFromCmdLine() noexcept;
    void addWad(const std::string& path) noexcept;
    bool hasAny() noexcept;
    bool hasMap(uint32_t mapNum) noexcept;
    bool mapHasAllLumps(uint32_t mapNum) noexcept;
    const uint8_t* getMapLump(uint32_t mapNum, uint32_t mlIndex, uint32_t& outSize) noexcept;
    const char* lastPreflightError() noexcept;
}
