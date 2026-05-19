#ifndef SHELLANG_FILEREAD_H
#define SHELLANG_FILEREAD_H

#include "ICmd.h"

struct FileRead : ICmd {
    int Run(const std::vector<std::string>& args, Context& context, std::istream& is, std::ostream& os) override {
        const std::string flags = getFlags(args);
        bool printCode = false;

        for(const char c : flags)
            switch(c) {
                case 'c': printCode = true; break;
                default: break;
            }

        std::vector<std::string> dataArgs = getDataArgs(args);

        for(const std::string& dataArg : dataArgs) {
            std::filesystem::path path = resolvePath(dataArg);

            if (std::filesystem::is_regular_file(path)) {
                if(printCode)
                    os << "```" << path.filename().string() << '\n';

                std::ifstream is (path.string());
                os << is.rdbuf() << '\n';
                is.close();

                if(printCode)
                    os << "```\n";
            }
        }

        return 0;
    }


};

#endif
