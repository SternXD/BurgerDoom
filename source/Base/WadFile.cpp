#include "WadFile.h"
#include <cctype>
#include <cstring>
#include <fstream>

/* atsb: more mess again

what we're doing isn't really loading a WAD, we're 'faking' balancing
the lookup for a REZFILE content of the lump we're reading
with what we're passing to the -file parameter

basically we pass it a wad, it'll treat it as LUMPS from its REZFILE
lookup the blockmap etc..  and then load the map..

mess....
*/

static inline int ci_cmp(const char* a, const char* b) {
    for (int i=0;i<8;i++) {
        unsigned char ca = (unsigned char)std::toupper((unsigned char)a[i]);
        unsigned char cb = (unsigned char)std::toupper((unsigned char)b[i]);
        if (ca!=cb) return (ca<cb)?-1:1;
        if (a[i]==0 && b[i]==0) return 0;
    }
    return 0;
}

static inline bool isMapMarkerName(const char* nm) {
    if (!nm) return false;
    if (nm[0]=='E' && nm[2]=='M' && nm[4]==0 &&
        (nm[1]>='0' && nm[1]<='9') && (nm[3]>='0' && nm[3]<='9'))
        return true;
    if (nm[0]=='M' && nm[1]=='A' && nm[2]=='P' && nm[5]==0 &&
        (nm[3]>='0' && nm[3]<='9') && (nm[4]>='0' && nm[4]<='9'))
        return true;
    return false;
}

WadFile::WadFile() noexcept {}
WadFile::~WadFile() noexcept {}

struct Header {
    char id[4];
    int32_t numlumps;
    int32_t infotableofs;
} hdr{};

bool WadFile::open(const std::string& path) noexcept {
    std::ifstream f(path, std::ios::binary);
    if (!f)
        return false;

    f.read((char*)&hdr, sizeof(hdr));
    if (!f)
        return false;
    if (std::memcmp(hdr.id,"IWAD",4)!=0 && std::memcmp(hdr.id,"PWAD",4)!=0)
        return false;
    if (hdr.numlumps<=0 || hdr.infotableofs<=0)
        return false;

    f.seekg(0, std::ios::end);
    size_t fileSize = (size_t)f.tellg();
    mData.resize(fileSize);
    f.seekg(0, std::ios::beg);
    f.read((char*)mData.data(), fileSize);

    if (!f) {
        mData.clear();
    return false;

    }
    mDir.resize(hdr.numlumps);
    const uint8_t* dirp = mData.data() + hdr.infotableofs;
    for (int i=0;i<hdr.numlumps;i++) {
        int32_t filepos = *(const int32_t*)(dirp + i*16 + 0);
        int32_t size    = *(const int32_t*)(dirp + i*16 + 4);
        const char* name= (const char*)(dirp + i*16 + 8);
        WadLump& L = mDir[i];
        L.offset = (uint32_t)filepos;
        L.size   = (uint32_t)size;
        std::memset(L.name,0,sizeof(L.name));
        std::memcpy(L.name,name,8);
    }
    return true;
}

int WadFile::findLumpIndex(const std::string& name) const noexcept {
    char temp[9]; std::memset(temp,0,sizeof(temp));
    std::strncpy(temp, name.c_str(), 8);
    for (size_t i=0;i<mDir.size();++i) {
        if (ci_cmp(mDir[i].name, temp)==0)
            return (int)i;
    }
    return -1;
}

const WadLump* WadFile::getLump(int index) const noexcept {
    if (index<0 || (size_t)index>=mDir.size())
        return nullptr;
    return &mDir[index];
}

const uint8_t* WadFile::getLumpData(int index) const noexcept {
    const WadLump* L = getLump(index);
    if (!L)
        return nullptr;
    if (L->offset + L->size > mData.size())
        return nullptr;
    return mData.data() + L->offset;
}

int WadFile::findMapMarker(const std::string& mapName) const noexcept {
    int idx = findLumpIndex(mapName);
    if (idx>=0)
        return idx;
    return -1;
}

int WadFile::findMapSubLump(int markerIndex, const char* subName) const noexcept {
    if (markerIndex<0) return -1;
    for (size_t i = (size_t)markerIndex+1; i<mDir.size(); ++i) {
        const char* nm = mDir[i].name;
        if (isMapMarkerName(nm))
            break; // stop at next map marker only
        if (ci_cmp(mDir[i].name, subName)==0)
            return (int)i;
    }
    return -1;
}
