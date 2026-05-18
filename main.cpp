#include <fstream>
#include <iostream>

#include "Shell.h"

#include <sstream>

#include "Clear.h"
#include "PathPrint.h"
#include "Echo.h"
#include "Exit.h"
#include "PathChange.h"
#include "UniversalList.h"

int main(const int argc, const char* argv[]) {
    Context context {{
        { "pp", std::make_shared<PathPrint>() },
        { "cp", std::make_shared<Echo>() },
        { "ce", std::make_shared<Exit>() },
        { "cc", std::make_shared<Clear>() },
        { "pc", std::make_shared<PathChange>() },
        { "ul", std::make_shared<UniversalList>() }
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
