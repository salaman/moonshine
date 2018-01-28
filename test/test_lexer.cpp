#include <catch/catch.hpp>

#include <moonshine/lexer/Lexer.h>
#include <moonshine/lexer/TokenType.h>

#include <sstream>

#define REQUIRE_TOKEN(TYPE, VALUE) \
    REQUIRE((token = lex.getNextToken()) != nullptr); \
    REQUIRE(token->type == (TYPE)); \
    REQUIRE(token->value == (VALUE)); \
    delete token

#define REQUIRE_EOF() \
    REQUIRE((token = lex.getNextToken()) == nullptr)

#define TEST_LEXER(EXPR, REQS) \
    TEST_CASE((EXPR), "[lexer]") { \
        Lexer lex; \
        std::istringstream stream((EXPR)); \
        lex.startLexing(&stream); \
        Token* token = nullptr; \
        REQS \
        REQUIRE_EOF(); \
    }

using namespace moonshine;

TEST_LEXER("0123", \
    REQUIRE_TOKEN(TokenType::T_INTEGER_LITERAL, "0");
    REQUIRE_TOKEN(TokenType::T_INTEGER_LITERAL, "123");
)

TEST_LEXER("01.23", \
    REQUIRE_TOKEN(TokenType::T_INTEGER_LITERAL, "0");
    REQUIRE_TOKEN(TokenType::T_FLOAT_LITERAL, "1.23");
)

TEST_LEXER("12.340", \
    REQUIRE_TOKEN(TokenType::T_FLOAT_LITERAL, "12.34");
    REQUIRE_TOKEN(TokenType::T_INTEGER_LITERAL, "0");
)

TEST_LEXER("012.340", \
    REQUIRE_TOKEN(TokenType::T_INTEGER_LITERAL, "0");
    REQUIRE_TOKEN(TokenType::T_FLOAT_LITERAL, "12.34");
    REQUIRE_TOKEN(TokenType::T_INTEGER_LITERAL, "0");
)

TEST_LEXER("12345", \
    REQUIRE_TOKEN(TokenType::T_INTEGER_LITERAL, "12345");
)

TEST_LEXER("abc", \
    REQUIRE_TOKEN(TokenType::T_IDENTIFIER, "abc");
)

TEST_LEXER("abc1", \
    REQUIRE_TOKEN(TokenType::T_IDENTIFIER, "abc1");
)

TEST_LEXER("abc_1", \
    REQUIRE_TOKEN(TokenType::T_IDENTIFIER, "abc_1");
)

TEST_LEXER("abc1_", \
    REQUIRE_TOKEN(TokenType::T_IDENTIFIER, "abc1_");
)

TEST_LEXER("_abc1", \
    REQUIRE_TOKEN(TokenType::T_NONE, "_");
    REQUIRE_TOKEN(TokenType::T_IDENTIFIER, "abc1");
)

TEST_LEXER("1abc", \
    REQUIRE_TOKEN(TokenType::T_INTEGER_LITERAL, "1");
    REQUIRE_TOKEN(TokenType::T_IDENTIFIER, "abc");
)

TEST_LEXER("_1abc", \
    REQUIRE_TOKEN(TokenType::T_NONE, "_");
    REQUIRE_TOKEN(TokenType::T_INTEGER_LITERAL, "1");
    REQUIRE_TOKEN(TokenType::T_IDENTIFIER, "abc");
)