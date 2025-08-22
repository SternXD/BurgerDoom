#pragma once
#include <cstdint>
#include <string>
#include <vector>

struct WadLump {
    uint32_t offset;
    uint32_t size;
    char     name[9];
};

class WadFile {
public:
    WadFile() noexcept;
    ~WadFile() noexcept;
    bool open(const std::string& path) noexcept;
    bool isOpen() const noexcept { return !mData.empty(); }

    int  findLumpIndex(const std::string& name) const noexcept;
    const WadLump* getLump(int index) const noexcept;
    const uint8_t* getLumpData(int index) const noexcept;

    int  findMapMarker(const std::string& mapName) const noexcept;
    int  findMapSubLump(int mapMarkerIndex, const char* subName) const noexcept;
private:
    std::vector<uint8_t> mData;
    std::vector<WadLump> mDir;
};
