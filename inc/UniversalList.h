#ifndef SHELLANG_UNIVERSALLIST_H
#define SHELLANG_UNIVERSALLIST_H

#include "ICmd.h"

struct UniversalList : ICmd {
    int Run(const std::vector<std::string>& args, Context& context, std::istream& is, std::ostream& os) override {
        const std::string flags = getFlags(args);
        bool showSize = false, doRecursive = false, separateNewline = false;

        for(const char c : flags)
            switch(c) {
                case 's': showSize = true; break;
                case 'r': doRecursive = true; break;
                case 'n': separateNewline = true; break;
                default: break;
            }

        std::vector<std::string> dataArgs = getDataArgs(args);

        if(dataArgs.empty())
            dataArgs.push_back(context.CurrentPath);

        const auto handle = [&](const std::filesystem::directory_entry& e){
            if(e.is_regular_file()) {
                os << e.path().filename().string();

                if(showSize)
                    os << ' ' << getReadableSize(getFileSize(e.path()));

                os << (separateNewline ? '\n' : ' ');
            }

            if(e.is_directory()) {
                os << e.path().filename().string() << '/';

                if(showSize)
                    os << ' ' << getReadableSize(getDirectorySize((e.path())));

                os << (separateNewline ? '\n' : ' ');
            }
        };

        for(const std::string& dataArg : dataArgs)
            if(!doRecursive)
                for(const auto& e : std::filesystem::directory_iterator(dataArg))
                    handle(e);
            else
                for(const auto& e : std::filesystem::recursive_directory_iterator(dataArg))
                    handle(e);

        return 0;
    }


};

#endif
