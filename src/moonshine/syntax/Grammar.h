#pragma once

#include "moonshine/lexer/TokenType.h"

#include <nlohmann/json.hpp>
#include <string>
#include <utility>
#include <vector>

namespace moonshine { namespace syntax {

enum class GrammarTokenType
{
    END,
    TERMINAL,
    NON_TERMINAL,
    SEMANTIC,
};

struct GrammarToken
{
    GrammarToken(const GrammarTokenType& type_, const int& value_)
        : type(type_), value(value_)
    {
    }

    const GrammarTokenType type;
    const int value;
};

struct Production
{
    Production()
        : Production(false, false)
    {
    }

    explicit Production(std::vector<GrammarToken> rhs_)
        : rhs(std::move(rhs_)), isScanError(false), isPopError(false)
    {
    }

    Production(const bool& scanError, const bool& popError)
        : rhs(), isScanError(scanError), isPopError(popError)
    {
    }

    const std::vector<GrammarToken> rhs;
    const bool isScanError;
    const bool isPopError;

    inline const bool isError() const
    {
        return isScanError || isPopError;
    }
};

class Grammar
{
public:
    Grammar(const char* grammarFileName, const char* tableFileName);
    std::string tokenName(const GrammarToken& token) const;
    GrammarToken startToken() const;

    const Production operator()(const GrammarToken& nonTerminal, const TokenType& input) const;
private:
    /**
     * Productions master list, indexed
     *
     * A vector entry corresponds to a row in the parse table, which stores a vector of tokens
     * forming the production's right-hand side.
     */
    std::vector<Production> productions_;

    /**
     * Non-terminal name list, indexed
     *
     * A vector entry corresponds to a row in the parse table, which stores the name of the non-terminal.
     */
    std::map<std::string, int> nonTerminals_;

    std::map<int, std::vector<GrammarToken>> first_;
    std::map<int, std::vector<GrammarToken>> follow_;
    std::vector<bool> nullable_;

    /**
     * Map of terminal token names to their enum value
     */
    std::map<std::string, TokenType> tokenLookup_;

    /**
     * Raw JSON predictive parser table
     */
    nlohmann::json table_;

    void createTokenLookup();
};

}}
