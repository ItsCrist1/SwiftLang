#include <iostream>

#include "Lexer.h"
#include "Parser.h"
#include "Evaluator.h"

#include <sstream>

#include "PathPrint.h"
#include "Echo.h"

int main(const int argc, const char* argv[]) {
    std::ostringstream oss;

    for(int i=1; i < argc; ++i)
        oss << argv[i] << ' ';

    Lexer lexer;
    const LexerOutput lexedTokens = lexer.Lex("eh < test.txt");

    Parser parser;
    const ParserOutput rootNode = parser.Parse(std::get<std::vector<Token>>(lexedTokens));

    Context context {
        {
            { "pp", std::make_shared<PathPrint>() },
            { "eh", std::make_shared<Echo>() }
        }
    };

    Evaluator evaluator(context);
    evaluator.Evaluate(std::get<RootNode>(rootNode));

    return 0;
}
