#include <fstream>
#include <iostream>

#include "Shell.h"

#include <sstream>

#include "ConsoleClear.h"
#include "PathPrint.h"
#include "ConsolePrint.h"
#include "ConsoleExit.h"
#include "PathChange.h"
#include "UniversalList.h"
#include "FileRead.h"

int main(const int argc, const char* argv[]) {
    Context context {{
        { "pp", std::make_shared<PathPrint>() },
        { "pc", std::make_shared<PathChange>() },

        { "cp", std::make_shared<ConsolePrint>() },
        { "ce", std::make_shared<ConsoleExit>() },
        { "cc", std::make_shared<ConsoleClear>() },

        { "fr", std::make_shared<FileRead>() },

        { "ul", std::make_shared<UniversalList>() },
    }};

    Shell shell (context);

    if(argc == 1)
        return shell.StartREPL();

    if(argc == 3 && std::string_view(argv[1]) == "--file") {
        std::ifstream file(argv[2]);
        if (!file) {
            std::cerr << "Error: cannot open file '" << argv[2] << "'\n";
            return 1;
        }
        std::ostringstream oss;
        oss << file.rdbuf();
        return shell.Evaluate(oss.str());
    }

    std::ostringstream oss;

    for(int i=1; i < argc; ++i)
        oss << argv[i] << ' ';

    return shell.Evaluate(oss.str());
}
