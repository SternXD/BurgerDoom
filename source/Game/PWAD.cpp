#include "PWAD.h"
#include "Base/WadFile.h"
#include "Base/CmdLine.h"
#include <memory>
#include <cstdio>
#include <vector>
#include <string>
#include <cstdint>

// atsb: dirty hack of a PWAD reader that internally
// teases the engine to think it is a lump of a REZFILE..

// so far it has gotten the best of me! :D

// really messy, but hey, I'm basically attempting to read the structure of a 'fake' REZFILE, gimme a break :D

namespace PWAD {

static std::vector<std::unique_ptr<WadFile>> gWads;
static std::string gLastError;
static std::string toMAP(uint32_t n) {
    char s[9]; std::snprintf(s,sizeof(s),"MAP%02u", n?n:1);
    return s;
}
static std::string toEEMM(uint32_t n) {
    if(!n)n=1;
    char s[9];
    std::snprintf(s,sizeof(s),"E%uM%u",(n-1)/9+1,(n-1)%9+1);
    return s;
}

void initFromCmdLine() noexcept {
    auto vals = CmdLine::getValues("file");
    for (auto& v : vals) addWad(v);
}
void addWad(const std::string& path) noexcept {
    auto w = std::make_unique<WadFile>();
    if (w->open(path)) gWads.emplace_back(std::move(w));
}
bool hasAny() noexcept {
    return !gWads.empty();
}

static int findMapMarkerInAny(const std::string& name) {
    for (int i=(int)gWads.size()-1;i>=0;--i) {
        int idx = gWads[i]->findMapMarker(name);
        if (idx>=0)
            return (i<<24)|(idx&0x00FFFFFF);
    }
    return -1;
}

static const char* kMlNames[10] = {
    "THINGS","LINEDEFS","SIDEDEFS","VERTEXES","SEGS",
    "SSECTORS","SECTORS","NODES","REJECT","BLOCKMAP"
};

static inline bool validLumpSize(uint32_t ml, uint32_t size) {
    switch(ml){
        case 0:  return size>0 && (size%10)==0; // THINGS
        case 1:  return size>0 && (size%14)==0; // LINEDEFS
        case 2:  return size>0 && (size%30)==0; // SIDEDEFS
        case 3:  return size>0 && (size%4 )==0; // VERTEXES
        case 4:  return size>0 && (size%12)==0; // SEGS
        case 5:  return size>0 && (size%4 )==0; // SSECTORS
        case 6:  return size>0 && (size%26)==0; // SECTORS
        case 7:  return size>0 && (size%28)==0; // NODES
        case 8:  return true;                   // REJECT (zero ok)
        case 9:  return size>0;                 // BLOCKMAP
        default: return false;
    }
}

static int findPackedMarker(uint32_t mapNum){
    int p = findMapMarkerInAny(toMAP(mapNum));
    if (p>=0)
        return p;
    return findMapMarkerInAny(toEEMM(mapNum));
}

const uint8_t* getMapLump(uint32_t mapNum, uint32_t mlIndex, uint32_t& outSize) noexcept {
    outSize = 0;
    if (gWads.empty() || mlIndex>=10)
        return nullptr;

    int packed = findPackedMarker(mapNum);
    if (packed<0)
        return nullptr;

    int wadIdx = (packed>>24)&0xFF;
    int marker =  packed     &0x00FFFFFF;

    int sub = gWads[wadIdx]->findMapSubLump(marker, kMlNames[mlIndex]);
    if (sub<0)
        return nullptr;

    const WadLump* L = gWads[wadIdx]->getLump(sub);
    if (!L)
        return nullptr;

    outSize = L->size;
    if (!validLumpSize(mlIndex, outSize)) {
        outSize=0;
        return nullptr;
    }
    return gWads[wadIdx]->getLumpData(sub);
}

bool hasMap(uint32_t mapNum) noexcept {
    uint32_t sz=0;
    return getMapLump(mapNum,0,sz) && getMapLump(mapNum,1,sz);
}

static inline uint16_t rd16u(const uint8_t* p){return (uint16_t)(p[0]|(p[1]<<8));}
static inline int16_t  rd16s(const uint8_t* p){return (int16_t)(p[0]|(p[1]<<8));}

static bool preflight(uint32_t map, std::string& err) {
    uint32_t sz[10]={0}; const uint8_t* p[10]={};
    for (int i=0;i<10;i++) {
        p[i]=getMapLump(map,i,sz[i]);
        if (!p[i] && i!=8) {
            err = std::string("missing/invalid ")+kMlNames[i];
            return false;
        }
    }
    int nThings=sz[0]/10, nLines=sz[1]/14, nSides=sz[2]/30, nVerts=sz[3]/4,
        nSegs=sz[4]/12, nSS=sz[5]/4, nSecs=sz[6]/26, nNodes=sz[7]/28;

    (void)nThings;

    for(int i=0;i<nSides;i++){
        int sec=rd16u(p[2]+i*30+28);
    if(sec<0||sec>=nSecs) {
        err="SIDEDEFS.sectorIndex";
        return false;
    } 
    }
    for(int i=0;i<nLines;i++) {
        const uint8_t* L=p[1]+i*14;
        int v1=rd16u(L+0), v2=rd16u(L+2);
        int rs=rd16s(L+10), ls=rd16s(L+12);
        if(v1<0||v1>=nVerts||v2<0||v2>=nVerts){
            err="LINEDEFS.v";
            return false;
        }
        if(rs!=-1&&(rs<0||rs>=nSides)){
            err="LINEDEFS.rightSide";
            return false;
        }
        if(ls!=-1&&(ls<0||ls>=nSides)){
            err="LINEDEFS.leftSide";
            return false;
        }
    }
    for(int i=0;i<nSegs;i++) {
        const uint8_t* S=p[4]+i*12;
        int v1=rd16u(S+0), v2=rd16u(S+2), ld=rd16u(S+8);
        if(v1<0||v1>=nVerts||v2<0||v2>=nVerts){
            err="SEGS.v";
            return false;
        }
        if(ld<0||ld>=nLines) {
            err="SEGS.linedef";
            return false;
        }
    }
    for(int i=0;i<nSS;i++) {
        const uint8_t* S=p[5]+i*4; int n=rd16u(S+0), fs=rd16u(S+2);
        if(n<1||fs<0||fs+n>nSegs){
            err="SSECTORS.range";
            return false;
        }
    }
    for(int i=0;i<nNodes;i++) {
        const uint8_t* N=p[7]+i*28; uint16_t c0=rd16u(N+24), c1=rd16u(N+26);
        auto ok=[&](uint16_t c){
            if(c&0x8000){
                return (int)(c&0x7FFF)<nSS;
            }
            return (int)c<nNodes;
            };
        if(!ok(c0)||!ok(c1)){
            err="NODES.child";
            return false;
        }
    }
    if (sz[9]>0){
        const uint8_t* B=p[9];
        if(sz[9]<8){
            err="BLOCKMAP small";
            return false;
        }
        int w=rd16u(B+4), h=rd16u(B+6); if(w<=0||h<=0){err="BLOCKMAP dims";return false;}
        int min=8 + (w*h)*2;
        if((int)sz[9]<min) {
            err="BLOCKMAP offsets";
            return false;
        }
    }
    if (sz[8]>0){
        int need=((nSecs*nSecs)+7)>>3;
        if((int)sz[8]<need) {
            err="REJECT size";
            return false;
        }
    }
    return true;
}

const char* lastPreflightError() noexcept { return gLastError.c_str(); }

bool mapHasAllLumps(uint32_t map) noexcept {
    gLastError.clear();
    if (gWads.empty()){
        gLastError="no PWADs";
    return false;
    }
    std::string err;
    if(!preflight(map, err)) {
        gLastError=err;
        return false;
    }
    return true;
}

}
