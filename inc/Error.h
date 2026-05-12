#ifndef SHELLANG_ERROR_H
#define SHELLANG_ERROR_H

#include <string>
#include <string_view>

class Error {
protected:
    std::string ErrorMessage;
    size_t x, y;

public:
    virtual ~Error() = default;

    explicit Error(std::string_view, size_t, size_t);

    [[nodiscard]] virtual std::string GetError() const = 0;
};

struct LexerError : Error {
    explicit LexerError(std::string_view, size_t, size_t);

    [[nodiscard]] std::string GetError() const override;
};

struct ParserError : Error {
    explicit ParserError(std::string_view, size_t, size_t);

    [[nodiscard]] std::string GetError() const override;
};

struct EvaluatorError : Error {
    explicit EvaluatorError(std::string_view, size_t, size_t);

    [[nodiscard]] std::string GetError() const override;
};

#endif
