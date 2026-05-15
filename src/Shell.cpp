#include "Shell.h"

Shell::Shell(Context& context) : context(std::make_unique<Context>(context)), evaluator(*this->context) {}

int Shell::Evaluate(const std::string& str, std::optional<Context> context) {
    if(context)
        this->context = std::make_unique<Context>(*context);

    const LexerOutput lo = lexer.Lex(str);

    if(std::holds_alternative<LexerError>(lo)) {
        std::cerr << std::get<LexerError>(lo).GetError() << std::endl;
        return -3;
    }

    const ParserOutput po = parser.Parse(std::get<std::vector<Token>>(lo));

    if(std::holds_alternative<ParserError>(po)) {
        std::cerr << std::get<ParserError>(po).GetError() << std::endl;
        return -2;
    }

    const EvaluatorOutput eo = evaluator.Evaluate(std::get<RootNode>(po));

    if(std::holds_alternative<EvaluatorError>(eo)) {
        std::cerr << std::get<EvaluatorError>(eo).GetError() << std::endl;
        return -1;
    }

    return std::get<int>(eo);
}

int Shell::StartREPL(std::optional<Context> context) {
    if(context)
        this->context = std::make_unique<Context>(*context);

    while(true) {
        std::cout << "\n[" << this->context->CurrentPath << "] ";

        std::string inp;

        if(!std::getline(std::cin, inp) || Evaluate(inp) == ICmd::EXIT_CODE)
            return 0;
    }
}
