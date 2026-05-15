#include <iostream>

#include "Shell.h"

#include <sstream>

#include "Clear.h"
#include "PathPrint.h"
#include "Echo.h"
#include "Exit.h"

int main(const int argc, const char* argv[]) {
    Context context {{
        { "pp", std::make_shared<PathPrint>() },
        { "eh", std::make_shared<Echo>() },
        { "ce", std::make_shared<Exit>() },
        { "cc", std::make_shared<Clear>() }
    }};

    Shell shell (context);

    if(argc == 1)
        return shell.StartREPL();

    std::ostringstream oss;

    for(int i=1; i < argc; ++i)
        oss << argv[i] << ' ';

    return shell.Evaluate(oss.str());
}
