#include "ICmd.h"

const std::string ICmd::resolvePath(std::string path) {
    if(path.empty() || path[0] == '~')
        path = std::string(getenv("HOME")) + (path.empty() ? "" : path.substr(1));
    
    return std::filesystem::canonical(path).string();
}
