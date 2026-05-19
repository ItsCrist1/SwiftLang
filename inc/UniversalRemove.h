#ifndef SHELLANG_UNIVERSALREMOVE_H
#define SHELLANG_UNIVERSALREMOVE_H

#include <algorithm>

#include "ICmd.h"

struct UniversalRemove : ICmd {
    int Run(const std::vector<std::string>& args, Context& context, std::istream& is, std::ostream& os) override {
        const std::string flags = getFlags(args);
        bool deleteDirectories = false, deleteFiles = true;

        for(const char c : flags)
            switch(c) {
                case 'r': deleteDirectories = true; break;
                case 'n': deleteFiles = false; break;
                default: break;
            }

        std::vector<std::string> dataArgs = getDataArgs(args);

        for(const std::string& dataArg : dataArgs) {
            std::filesystem::path path = resolvePath(dataArg);

            if(std::filesystem::is_regular_file(path) && deleteFiles)
                std::filesystem::remove(path);

            if(std::filesystem::is_directory(path) && deleteDirectories) {
                if(std::ranges::contains(RISKY_PATHS,dataArg)) {
                    os << "Are you sure you want to delete this? [y/N] ";
                    std::string answer;
                    is >> answer;

                    if(answer != "y")
                        continue;
                }

                std::filesystem::remove_all(path);
            }
        }

        return 0;
    }

private:
    const std::vector<std::string> RISKY_PATHS = {
        "/",
        "/*",
        "~",
        ".",
        "..",
        "*",
        "./",
        "~/",
        "~/*"
    };

};

#endif
