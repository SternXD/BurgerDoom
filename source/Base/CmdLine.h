#pragma once
#include <vector>
#include <string>
namespace CmdLine {
    void init(int argc, char** argv) noexcept;
    const std::vector<std::string>& args() noexcept;
    bool hasFlag(const char* name) noexcept;
    std::vector<std::string> getValues(const char* name) noexcept;
}
