#ifndef SHELLANG_UNIVERSALCOPY_H
#define SHELLANG_UNIVERSALCOPY_H

#include <algorithm>

#include "ICmd.h"

struct UniversalCopy : ICmd {
    int Run(const std::vector<std::string>& args, Context& context, std::istream& is, std::ostream& os) override {
        const std::string flags = getFlags(args);
        bool doRecursive = false;

        for(const char c : flags)
            switch(c) {
                case 'r': doRecursive = true; break;
                default: break;
            }

        std::vector<std::string> dataArgs = getDataArgs(args);

        if(dataArgs.size() != 2)
            return 1;

        const std::filesystem::path source = resolvePath(dataArgs[0]),
                                    target = resolvePath(dataArgs[1]);


        if(std::filesystem::is_regular_file(source))
            std::filesystem::copy_file(source, target);
        else if(std::filesystem::is_directory(source) && doRecursive)
            std::filesystem::copy(source, target, std::filesystem::copy_options::recursive | std::filesystem::copy_options::overwrite_existing);

        return 0;
    }
};

#endif
