#include "CmdLine.h"
#include <cstring>
namespace CmdLine {
    static std::vector<std::string> gArgs;
    void init(int argc, char** argv) noexcept {
        gArgs.clear();
        for (int i=0;i<argc;i++) gArgs.emplace_back(argv[i] ? argv[i] : "");
    }
    const std::vector<std::string>& args() noexcept {
        return gArgs;
    }
    static bool isFlag(const std::string& s) {
        return !s.empty() && s[0]=='-' && (s.size()==1 || s[1]!='-');
    }

    bool hasFlag(const char* name) noexcept {
        if (!name)
            return false;

        const std::string flag = std::string("-")+name;
        for (size_t i=1;i<gArgs.size();++i) if (gArgs[i]==flag)
            return true;
        return false;
    }
    std::vector<std::string> getValues(const char* name) noexcept {
        std::vector<std::string> out;
        if (!name)
            return out;
        const std::string flag = std::string("-")+name;
        for (size_t i=1;i<gArgs.size();++i) {
            if (gArgs[i]==flag) {
                for (size_t j=i+1;j<gArgs.size();++j) {
                    if (isFlag(gArgs[j]))
                        break;
                    out.push_back(gArgs[j]);
                }
                break;
            }
        }
        return out;
    }
}
