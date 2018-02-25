#include "Grammar.h"

#include <fstream>
#include <sstream>
#include <iostream>
#include <stdexcept>
#include <algorithm>

namespace moonshine { namespace syntax {

using json = nlohmann::json;

Grammar::Grammar(const char* grammarFileName, const char* tableFileName)
{
    createTokenLookup();

    std::string line;
    std::string temp;

    /*
     * Process parse table
     */

    // read json
    {
        std::ifstream tableFile(tableFileName);
        tableFile >> table_;
        tableFile.close();
    }

    //auto nonTerminals = table.size() - 2;
    //std::cout << nonTerminals << std::endl;

    /*
     * Pre-process non-terminals
     */

    std::ifstream grammarFile(grammarFileName);
    int nonTerminalId = 0;

    while (std::getline(grammarFile, line)) {
        std::istringstream iss(line);
        iss >> temp;

        // skip over empties and non-terminals we've already seen
        if (temp.empty() || nonTerminals_.find(temp) != nonTerminals_.end()) {
            continue;
        }

        // save this new non-terminal
        nonTerminals_[temp] = ++nonTerminalId;
    }

    for (const auto& i : nonTerminals_) {
        std::cout << i.second << ": " << i.first << std::endl;
    }

    std::cout << "Parsed " << nonTerminals_.size() << " non-terminals" << std::endl;

    // reset file
    grammarFile.clear();
    grammarFile.seekg(0, std::ios::beg);

    /*
     * Process productions
     */

    // insert dummy production at pos 0 since table is 1-indexed
    productions_.emplace_back();

    while (std::getline(grammarFile, line)) {
        std::istringstream iss(line);

        //auto currentIndex = productions_.size();

        std::string nonterminal;

        // parse the LHS (non-terminal)
        iss >> nonterminal;

        if (nonterminal.empty()) {
            continue;
        }

        iss >> temp; // ->

        std::vector<GrammarToken> tokens;

        std::cout << nonterminal << " " << temp << " ";

        while (iss >> temp) {

            auto nonTerminal = nonTerminals_.find(temp);
            auto terminal = tokenLookup_.find(temp);

            bool isNonTerminal = nonTerminal != nonTerminals_.end();
            bool isTerminal = terminal != tokenLookup_.end();
            bool isEpsilon = temp == "EPSILON";

            if (!isTerminal && !isNonTerminal && !isEpsilon) {
                throw std::runtime_error(std::string("Invalid token encountered: ") + temp);
            }

            std::cout << (isEpsilon ? "ε" : temp) << " ";

            if (isEpsilon) {
                continue;
            }

            GrammarTokenType type = isNonTerminal ? GrammarTokenType::NON_TERMINAL : GrammarTokenType::TERMINAL;
            int value = isNonTerminal ? nonTerminal->second : static_cast<int>(terminal->second);
            tokens.emplace_back(type, value);
        }

        productions_.emplace_back(tokens);

        std::cout << std::endl;
    }

    std::cout << "Parsed " << productions_.size() - 1 << " productions" << std::endl;

    grammarFile.close();

    /*
    * Process first sets
    */

    nullable_.insert(nullable_.begin(), static_cast<size_t>(nonTerminalId), false);

    std::ifstream firstFile("first.txt");

    while (std::getline(firstFile, line)) {
        std::istringstream iss(line);
        iss >> temp;

        // skip over empties
        if (temp.empty()) {
            continue;
        }

        auto nonTerminal = nonTerminals_.find(temp);

        if (nonTerminal == nonTerminals_.end()) {
            //throw std::runtime_error(std::string("Invalid non-terminal encountered in first file: ") + temp);
            continue;
        }

        // gobble up some whitespace (tab separator)
        iss >> std::ws;

        // extract the remainder of the line
        std::string s = iss.str().substr(static_cast<unsigned long>(iss.tellg()));

        std::vector<GrammarToken> firstTokens;
        std::string delim = ", ";
        std::string::size_type start = 0;
        std::string::size_type end = 0;

        for (; end <= s.length(); ++end) {
            // delimiter check
            if (end == s.length() || (s[end] == ',' && s[end + 1] == ' ')) {
                temp = s.substr(start, end - start);

                if (temp == "ε") {
                    nullable_[nonTerminal->second] = true;
                } else {
                    auto value = tokenLookup_.find(temp);

                    if (value == tokenLookup_.end()) {
                        throw std::runtime_error(std::string("Invalid terminal encountered in first file: ") + temp);
                    }

                    firstTokens.emplace_back(GrammarTokenType::TERMINAL, static_cast<int>(value->second));
                }

                end += 2;
                start = end;
            }
        }

        first_.emplace(std::make_pair(nonTerminal->second, firstTokens));

        //for (const auto& t : firstTokens) {
        //    std::cout << TokenName[static_cast<TokenType>(t.value)] << " | ";
        //}
        //std::cout << std::endl;
    }

    firstFile.close();

    /*
     * Process follow sets
     */

    std::ifstream followFile("follow.txt");

    while (std::getline(followFile, line)) {
        std::istringstream iss(line);
        iss >> temp;

        // skip over empties
        if (temp.empty()) {
            continue;
        }

        auto nonTerminal = nonTerminals_.find(temp);

        if (nonTerminal == nonTerminals_.end()) {
            throw std::runtime_error(std::string("Invalid non-terminal encountered in follow file: ") + temp);
        }

        // gobble up some whitespace (tab separator)
        iss >> std::ws;

        // extract the remainder of the line
        std::string s = iss.str().substr(static_cast<unsigned long>(iss.tellg()));

        std::vector<GrammarToken> followTokens;
        std::string delim = ", ";
        std::string::size_type start = 0;
        std::string::size_type end = 0;

        for (; end <= s.length(); ++end) {
            // delimiter check
            if (end == s.length() || (s[end] == ',' && s[end + 1] == ' ')) {
                temp = s.substr(start, end - start);

                if (temp != "$") {
                    auto value = tokenLookup_.find(temp);

                    if (value == tokenLookup_.end()) {
                        throw std::runtime_error(std::string("Invalid terminal encountered in follow file: ") + temp);
                    }

                    followTokens.emplace_back(GrammarTokenType::TERMINAL, static_cast<int>(value->second));
                }

                end += 2;
                start = end;
            }
        }

        follow_.emplace(std::make_pair(nonTerminal->second, followTokens));

        //for (const auto& t : followTokens) {
        //    std::cout << TokenName[static_cast<TokenType>(t.value)] << " | ";
        //}
        //std::cout << std::endl;
    }

    followFile.close();
}

void Grammar::createTokenLookup()
{
    for (const auto& i : TokenName) {
        tokenLookup_[i.second] = i.first;
    }
}

const Production Grammar::operator()(const GrammarToken& nonTerminal, const TokenType& input) const
{
    if (nonTerminal.type != GrammarTokenType::NON_TERMINAL) {
        throw std::invalid_argument("Given GrammarToken is not a non-terminal");
    }

    int col = 0;
    auto headerRow = table_[0];
    for (json::iterator it = headerRow.begin(); *it != TokenName[input] && it != headerRow.end(); ++it, ++col);

    auto nonTerminalRow = table_[nonTerminal.value + 1];
    const unsigned int productionId = nonTerminalRow[col];

    if (productionId == productions_.size()) {

        // -- hack for online parse table generator
        // the predict sets generated by the tool appear to be missing FOLLOW(A) if A is nullable, ie. ε ∈ FIRST(A)
        // this causes a pop error in the table, where instead it should be using a nullable production
        // this patch will override the pop error by checking to see if there exists a production A -> Bα such that ε ∈ FIRST(B)
        if (nullable_[nonTerminal.value]) {
            for (json::iterator it = ++nonTerminalRow.begin(); it != nonTerminalRow.end(); ++it) {
                // ignore error entries
                if (*it < productions_.size()) {
                    // check for ε ∈ FIRST(B)
                    if (productions_[*it].rhs.empty()
                        || (productions_[*it].rhs.front().type == GrammarTokenType::NON_TERMINAL && nullable_[productions_[*it].rhs.front().value])) {
                        return productions_[*it];
                    }
                }
            }
        }
        // -- end hack

        return Production(false, true);
    } else if (productionId == productions_.size() + 1) {
        return Production(true, false);
    }

    return productions_[productionId];
}

std::string Grammar::tokenName(const GrammarToken& token) const
{
    if (token.type == GrammarTokenType::TERMINAL) {
        return TokenName[static_cast<TokenType>(token.value)];
    } else if (token.type == GrammarTokenType::NON_TERMINAL) {
        return std::find_if(nonTerminals_.begin(), nonTerminals_.end(), [&token](const std::map<std::string, int>::value_type& i) {
            return i.second == token.value;
        })->first;
    } else if (token.type == GrammarTokenType::END) {
        return "$";
    }

    return {};
}

GrammarToken Grammar::startToken() const
{
    return GrammarToken(GrammarTokenType::NON_TERMINAL, 1);
}

}}
