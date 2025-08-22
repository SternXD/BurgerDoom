#include "Game/DoomMain.h"
#include "Base/CmdLine.h"

int main(int argc, char** argv) noexcept {
    CmdLine::init(argc, argv);
    D_DoomMain();
    return 0;
}
