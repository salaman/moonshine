#include <catch/catch.hpp>

#include <moonshine/lexer/Lexer.h>
#include <moonshine/lexer/TokenType.h>
#include <moonshine/lexer/ParseError.h>

#include <sstream>

#define REQUIRE_TOKEN(TYPE, VALUE, POS) \
    REQUIRE(token != nullptr); \
    REQUIRE(token->type == (TYPE)); \
    REQUIRE(token->value == (VALUE)); \
    REQUIRE(token->position == (POS)); \
    delete token; \
    token = lex.getNextToken() \

#define REQUIRE_EOF() REQUIRE(token == nullptr)

#define REQUIRE_NO_ERRORS() REQUIRE(lex.getErrors().empty())

#define REQUIRE_ERRORS(COUNT) \
    unsigned int __error_counter = 0; \
    REQUIRE(lex.getErrors().size() == (COUNT))

#define REQUIRE_ERROR(TYPE, VALUE, POS) \
    REQUIRE(lex.getErrors()[__error_counter].type == (TYPE)); \
    REQUIRE(lex.getErrors()[__error_counter].value == (VALUE)); \
    REQUIRE(lex.getErrors()[__error_counter++].position == (POS))

#define TEST_LEXER(EXPR, REQS) \
    TEST_CASE((EXPR), "[lexer]") { \
        Lexer lex; \
        std::istringstream stream((EXPR)); \
        lex.startLexing(&stream); \
        Token* token = lex.getNextToken(); \
        REQS \
        REQUIRE_EOF(); \
    }

using namespace moonshine;

/*
 * atoms
 */

TEST_LEXER("0123", \
    REQUIRE_TOKEN(TokenType::T_INTEGER_LITERAL, "0", 0);
    REQUIRE_TOKEN(TokenType::T_INTEGER_LITERAL, "123", 1);
    REQUIRE_NO_ERRORS();
)

TEST_LEXER("01.23", \
    REQUIRE_TOKEN(TokenType::T_INTEGER_LITERAL, "0", 0);
    REQUIRE_TOKEN(TokenType::T_FLOAT_LITERAL, "1.23", 1);
    REQUIRE_NO_ERRORS();
)

TEST_LEXER("12.340", \
    REQUIRE_TOKEN(TokenType::T_FLOAT_LITERAL, "12.34", 0);
    REQUIRE_TOKEN(TokenType::T_INTEGER_LITERAL, "0", 5);
    REQUIRE_NO_ERRORS();
)

TEST_LEXER("012.340", \
    REQUIRE_TOKEN(TokenType::T_INTEGER_LITERAL, "0", 0);
    REQUIRE_TOKEN(TokenType::T_FLOAT_LITERAL, "12.34", 1);
    REQUIRE_TOKEN(TokenType::T_INTEGER_LITERAL, "0", 6);
    REQUIRE_NO_ERRORS();
)

TEST_LEXER("12345", \
    REQUIRE_TOKEN(TokenType::T_INTEGER_LITERAL, "12345", 0);
    REQUIRE_NO_ERRORS();
)

TEST_LEXER("abc", \
    REQUIRE_TOKEN(TokenType::T_IDENTIFIER, "abc", 0);
    REQUIRE_NO_ERRORS();
)

TEST_LEXER("abc1", \
    REQUIRE_TOKEN(TokenType::T_IDENTIFIER, "abc1", 0);
    REQUIRE_NO_ERRORS();
)

TEST_LEXER("abc_1", \
    REQUIRE_TOKEN(TokenType::T_IDENTIFIER, "abc_1", 0);
    REQUIRE_NO_ERRORS();
)

TEST_LEXER("abc1_", \
    REQUIRE_TOKEN(TokenType::T_IDENTIFIER, "abc1_", 0);
    REQUIRE_NO_ERRORS();
)

TEST_LEXER("_abc1", \
    REQUIRE_TOKEN(TokenType::T_IDENTIFIER, "abc1", 1);
    REQUIRE_ERRORS(1);
    REQUIRE_ERROR(ParseErrorType::E_INVALID_CHARACTERS, "_", 0);
)

TEST_LEXER("1abc", \
    REQUIRE_TOKEN(TokenType::T_INTEGER_LITERAL, "1", 0);
    REQUIRE_TOKEN(TokenType::T_IDENTIFIER, "abc", 1);
    REQUIRE_NO_ERRORS();
)

TEST_LEXER("_1abc", \
    REQUIRE_TOKEN(TokenType::T_INTEGER_LITERAL, "1", 1);
    REQUIRE_TOKEN(TokenType::T_IDENTIFIER, "abc", 2);
    REQUIRE_ERRORS(1);
    REQUIRE_ERROR(ParseErrorType::E_INVALID_CHARACTERS, "_", 0);
)

TEST_LEXER("0", \
    REQUIRE_TOKEN(TokenType::T_INTEGER_LITERAL, "0", 0);
    REQUIRE_NO_ERRORS();
)

TEST_LEXER("0.0", \
    REQUIRE_TOKEN(TokenType::T_FLOAT_LITERAL, "0.0", 0);
    REQUIRE_NO_ERRORS();
)

TEST_LEXER("0.", \
    REQUIRE_TOKEN(TokenType::T_INTEGER_LITERAL, "0", 0);
    REQUIRE_TOKEN(TokenType::T_PERIOD, ".", 1);
    REQUIRE_NO_ERRORS();
)

TEST_LEXER(".0", \
    REQUIRE_TOKEN(TokenType::T_PERIOD, ".", 0);
    REQUIRE_TOKEN(TokenType::T_INTEGER_LITERAL, "0", 1);
    REQUIRE_NO_ERRORS();
)

/*
 * single tokens (operators, keywords, etc)
 */

TEST_LEXER("==", \
    REQUIRE_TOKEN(TokenType::T_IS_EQUAL, "==", 0);
    REQUIRE_NO_ERRORS();
)

TEST_LEXER("<>", \
    REQUIRE_TOKEN(TokenType::T_IS_NOT_EQUAL, "<>", 0);
    REQUIRE_NO_ERRORS();
)

TEST_LEXER("<", \
    REQUIRE_TOKEN(TokenType::T_IS_SMALLER, "<", 0);
    REQUIRE_NO_ERRORS();
)

TEST_LEXER(">", \
    REQUIRE_TOKEN(TokenType::T_IS_GREATER, ">", 0);
    REQUIRE_NO_ERRORS();
)

TEST_LEXER("<=", \
    REQUIRE_TOKEN(TokenType::T_IS_SMALLER_OR_EQUAL, "<=", 0);
    REQUIRE_NO_ERRORS();
)

TEST_LEXER(">=", \
    REQUIRE_TOKEN(TokenType::T_IS_GREATER_OR_EQUAL, ">=", 0);
    REQUIRE_NO_ERRORS();
)

TEST_LEXER(";", \
    REQUIRE_TOKEN(TokenType::T_SEMICOLON, ";", 0);
    REQUIRE_NO_ERRORS();
)

TEST_LEXER(",", \
    REQUIRE_TOKEN(TokenType::T_COMMA, ",", 0);
    REQUIRE_NO_ERRORS();
)

TEST_LEXER(".", \
    REQUIRE_TOKEN(TokenType::T_PERIOD, ".", 0);
    REQUIRE_NO_ERRORS();
)

TEST_LEXER(":", \
    REQUIRE_TOKEN(TokenType::T_COLON, ":", 0);
    REQUIRE_NO_ERRORS();
)

TEST_LEXER("::", \
    REQUIRE_TOKEN(TokenType::T_DOUBLE_COLON, "::", 0);
    REQUIRE_NO_ERRORS();
)

TEST_LEXER("+", \
    REQUIRE_TOKEN(TokenType::T_PLUS, "+", 0);
    REQUIRE_NO_ERRORS();
)

TEST_LEXER("-", \
    REQUIRE_TOKEN(TokenType::T_MINUS, "-", 0);
    REQUIRE_NO_ERRORS();
)

TEST_LEXER("*", \
    REQUIRE_TOKEN(TokenType::T_MUL, "*", 0);
    REQUIRE_NO_ERRORS();
)

TEST_LEXER("/", \
    REQUIRE_TOKEN(TokenType::T_DIV, "/", 0);
    REQUIRE_NO_ERRORS();
)

TEST_LEXER("=", \
    REQUIRE_TOKEN(TokenType::T_EQUAL, "=", 0);
    REQUIRE_NO_ERRORS();
)

TEST_LEXER("and", \
    REQUIRE_TOKEN(TokenType::T_AND, "and", 0);
    REQUIRE_NO_ERRORS();
)

TEST_LEXER("not", \
    REQUIRE_TOKEN(TokenType::T_NOT, "not", 0);
    REQUIRE_NO_ERRORS();
)

TEST_LEXER("or", \
    REQUIRE_TOKEN(TokenType::T_OR, "or", 0);
    REQUIRE_NO_ERRORS();
)

TEST_LEXER("(", \
    REQUIRE_TOKEN(TokenType::T_OPEN_PARENTHESIS, "(", 0);
    REQUIRE_NO_ERRORS();
)

TEST_LEXER(")", \
    REQUIRE_TOKEN(TokenType::T_CLOSE_PARENTHESIS, ")", 0);
    REQUIRE_NO_ERRORS();
)

TEST_LEXER("{", \
    REQUIRE_TOKEN(TokenType::T_OPEN_BRACE, "{", 0);
    REQUIRE_NO_ERRORS();
)

TEST_LEXER("}", \
    REQUIRE_TOKEN(TokenType::T_CLOSE_BRACE, "}", 0);
    REQUIRE_NO_ERRORS();
)

TEST_LEXER("[", \
    REQUIRE_TOKEN(TokenType::T_OPEN_BRACKET, "[", 0);
    REQUIRE_NO_ERRORS();
)

TEST_LEXER("]", \
    REQUIRE_TOKEN(TokenType::T_CLOSE_BRACKET, "]", 0);
    REQUIRE_NO_ERRORS();
)

TEST_LEXER("if", \
    REQUIRE_TOKEN(TokenType::T_IF, "if", 0);
    REQUIRE_NO_ERRORS();
)

TEST_LEXER("then", \
    REQUIRE_TOKEN(TokenType::T_THEN, "then", 0);
    REQUIRE_NO_ERRORS();
)

TEST_LEXER("else", \
    REQUIRE_TOKEN(TokenType::T_ELSE, "else", 0);
    REQUIRE_NO_ERRORS();
)

TEST_LEXER("for", \
    REQUIRE_TOKEN(TokenType::T_FOR, "for", 0);
    REQUIRE_NO_ERRORS();
)

TEST_LEXER("class", \
    REQUIRE_TOKEN(TokenType::T_CLASS, "class", 0);
    REQUIRE_NO_ERRORS();
)

TEST_LEXER("int", \
    REQUIRE_TOKEN(TokenType::T_INT, "int", 0);
    REQUIRE_NO_ERRORS();
)

TEST_LEXER("float", \
    REQUIRE_TOKEN(TokenType::T_FLOAT, "float", 0);
    REQUIRE_NO_ERRORS();
)

TEST_LEXER("get", \
    REQUIRE_TOKEN(TokenType::T_GET, "get", 0);
    REQUIRE_NO_ERRORS();
)

TEST_LEXER("put", \
    REQUIRE_TOKEN(TokenType::T_PUT, "put", 0);
    REQUIRE_NO_ERRORS();
)

TEST_LEXER("return", \
    REQUIRE_TOKEN(TokenType::T_RETURN, "return", 0);
    REQUIRE_NO_ERRORS();
)

TEST_LEXER("program", \
    REQUIRE_TOKEN(TokenType::T_PROGRAM, "program", 0);
    REQUIRE_NO_ERRORS();
)

/*
 * whitespace handling
 */

TEST_LEXER("   abc      ", \
    REQUIRE_TOKEN(TokenType::T_IDENTIFIER, "abc", 3);
    REQUIRE_NO_ERRORS();
)

TEST_LEXER("\r\n1 \n. \t2  \r3  abc\n", \
    REQUIRE_TOKEN(TokenType::T_INTEGER_LITERAL, "1", 2);
    REQUIRE_TOKEN(TokenType::T_PERIOD, ".", 5);
    REQUIRE_TOKEN(TokenType::T_INTEGER_LITERAL, "2", 8);
    REQUIRE_TOKEN(TokenType::T_INTEGER_LITERAL, "3", 12);
    REQUIRE_TOKEN(TokenType::T_IDENTIFIER, "abc", 15);
    REQUIRE_NO_ERRORS();
)

/*
 * ambiguity handling
 */

TEST_LEXER("12.3400000", \
    REQUIRE_TOKEN(TokenType::T_FLOAT_LITERAL, "12.34", 0);
    REQUIRE_TOKEN(TokenType::T_INTEGER_LITERAL, "0", 5);
    REQUIRE_TOKEN(TokenType::T_INTEGER_LITERAL, "0", 6);
    REQUIRE_TOKEN(TokenType::T_INTEGER_LITERAL, "0", 7);
    REQUIRE_TOKEN(TokenType::T_INTEGER_LITERAL, "0", 8);
    REQUIRE_TOKEN(TokenType::T_INTEGER_LITERAL, "0", 9);
    REQUIRE_NO_ERRORS();
)

TEST_LEXER("int abc", \
    REQUIRE_TOKEN(TokenType::T_INT, "int", 0);
    REQUIRE_TOKEN(TokenType::T_IDENTIFIER, "abc", 4);
    REQUIRE_NO_ERRORS();
)

TEST_LEXER("intabc", \
    REQUIRE_TOKEN(TokenType::T_IDENTIFIER, "intabc", 0);
    REQUIRE_NO_ERRORS();
)

TEST_LEXER("abc and def", \
    REQUIRE_TOKEN(TokenType::T_IDENTIFIER, "abc", 0);
    REQUIRE_TOKEN(TokenType::T_AND, "and", 4);
    REQUIRE_TOKEN(TokenType::T_IDENTIFIER, "def", 8);
    REQUIRE_NO_ERRORS();
)

TEST_LEXER("abcand def", \
    REQUIRE_TOKEN(TokenType::T_IDENTIFIER, "abcand", 0);
    REQUIRE_TOKEN(TokenType::T_IDENTIFIER, "def", 7);
    REQUIRE_NO_ERRORS();
)

TEST_LEXER("abc anddef", \
    REQUIRE_TOKEN(TokenType::T_IDENTIFIER, "abc", 0);
    REQUIRE_TOKEN(TokenType::T_IDENTIFIER, "anddef", 4);
    REQUIRE_NO_ERRORS();
)

TEST_LEXER("abcanddef", \
    REQUIRE_TOKEN(TokenType::T_IDENTIFIER, "abcanddef", 0);
    REQUIRE_NO_ERRORS();
)

TEST_LEXER("int int", \
    REQUIRE_TOKEN(TokenType::T_INT, "int", 0);
    REQUIRE_TOKEN(TokenType::T_INT, "int", 4);
    REQUIRE_NO_ERRORS();
)

TEST_LEXER("intint", \
    REQUIRE_TOKEN(TokenType::T_IDENTIFIER, "intint", 0);
    REQUIRE_NO_ERRORS();
)

TEST_LEXER("1 2 3", \
    REQUIRE_TOKEN(TokenType::T_INTEGER_LITERAL, "1", 0);
    REQUIRE_TOKEN(TokenType::T_INTEGER_LITERAL, "2", 2);
    REQUIRE_TOKEN(TokenType::T_INTEGER_LITERAL, "3", 4);
    REQUIRE_NO_ERRORS();
)

TEST_LEXER("12 3", \
    REQUIRE_TOKEN(TokenType::T_INTEGER_LITERAL, "12", 0);
    REQUIRE_TOKEN(TokenType::T_INTEGER_LITERAL, "3", 3);
    REQUIRE_NO_ERRORS();
)

TEST_LEXER("1 23", \
    REQUIRE_TOKEN(TokenType::T_INTEGER_LITERAL, "1", 0);
    REQUIRE_TOKEN(TokenType::T_INTEGER_LITERAL, "23", 2);
    REQUIRE_NO_ERRORS();
)

/*
 * float exponent notation
 */

TEST_LEXER("12.34e+0", \
    REQUIRE_TOKEN(TokenType::T_FLOAT_LITERAL, "12.34e+0", 0);
    REQUIRE_NO_ERRORS();
)

TEST_LEXER("12.34e+123", \
    REQUIRE_TOKEN(TokenType::T_FLOAT_LITERAL, "12.34e+123", 0);
    REQUIRE_NO_ERRORS();
)

TEST_LEXER("12.34e+00", \
    REQUIRE_TOKEN(TokenType::T_FLOAT_LITERAL, "12.34e+0", 0);
    REQUIRE_TOKEN(TokenType::T_INTEGER_LITERAL, "0", 8);
    REQUIRE_NO_ERRORS();
)

TEST_LEXER("12.34e+1.0", \
    REQUIRE_TOKEN(TokenType::T_FLOAT_LITERAL, "12.34e+1", 0);
    REQUIRE_TOKEN(TokenType::T_PERIOD, ".", 8);
    REQUIRE_TOKEN(TokenType::T_INTEGER_LITERAL, "0", 9);
    REQUIRE_NO_ERRORS();
)

TEST_LEXER("12.34e-0", \
    REQUIRE_TOKEN(TokenType::T_FLOAT_LITERAL, "12.34e-0", 0);
    REQUIRE_NO_ERRORS();
)

TEST_LEXER("12.34e-123", \
    REQUIRE_TOKEN(TokenType::T_FLOAT_LITERAL, "12.34e-123", 0);
    REQUIRE_NO_ERRORS();
)

/*
 * compound statements
 */

TEST_LEXER("int a=0;", \
    REQUIRE_TOKEN(TokenType::T_INT, "int", 0);
    REQUIRE_TOKEN(TokenType::T_IDENTIFIER, "a", 4);
    REQUIRE_TOKEN(TokenType::T_EQUAL, "=", 5);
    REQUIRE_TOKEN(TokenType::T_INTEGER_LITERAL, "0", 6);
    REQUIRE_TOKEN(TokenType::T_SEMICOLON, ";", 7);
    REQUIRE_NO_ERRORS();
)

TEST_LEXER("program {\nint a1_ = 123;\n}", \
    REQUIRE_TOKEN(TokenType::T_PROGRAM, "program", 0);
    REQUIRE_TOKEN(TokenType::T_OPEN_BRACE, "{", 8);
    REQUIRE_TOKEN(TokenType::T_INT, "int", 10);
    REQUIRE_TOKEN(TokenType::T_IDENTIFIER, "a1_", 14);
    REQUIRE_TOKEN(TokenType::T_EQUAL, "=", 18);
    REQUIRE_TOKEN(TokenType::T_INTEGER_LITERAL, "123", 20);
    REQUIRE_TOKEN(TokenType::T_SEMICOLON, ";", 23);
    REQUIRE_TOKEN(TokenType::T_CLOSE_BRACE, "}", 25);
    REQUIRE_NO_ERRORS();
)

TEST_LEXER("program {\n/*\nint a1_ = 123;\n*/\n}", \
    REQUIRE_TOKEN(TokenType::T_PROGRAM, "program", 0);
    REQUIRE_TOKEN(TokenType::T_OPEN_BRACE, "{", 8);
    REQUIRE_TOKEN(TokenType::T_CLOSE_BRACE, "}", 31);
    REQUIRE_NO_ERRORS();
)

/*
 * empty input
 */

TEST_LEXER("", REQUIRE_NO_ERRORS();)
TEST_LEXER(" ", REQUIRE_NO_ERRORS();)
TEST_LEXER("\r\n", REQUIRE_NO_ERRORS();)
TEST_LEXER("\n\r   \t \n \n", REQUIRE_NO_ERRORS();)

/*
 * comment handling
 */

TEST_LEXER("int //;a", \
    REQUIRE_TOKEN(TokenType::T_INT, "int", 0);
    REQUIRE_NO_ERRORS();
)

TEST_LEXER("int /*;*/a", \
    REQUIRE_TOKEN(TokenType::T_INT, "int", 0);
    REQUIRE_TOKEN(TokenType::T_IDENTIFIER, "a", 9);
    REQUIRE_NO_ERRORS();
)

TEST_LEXER("int //a\n123", \
    REQUIRE_TOKEN(TokenType::T_INT, "int", 0);
    REQUIRE_TOKEN(TokenType::T_INTEGER_LITERAL, "123", 8);
    REQUIRE_NO_ERRORS();
)

TEST_LEXER("_a/*x \r\n123* // / */b\n1 /;//*a*/\nc", \
    REQUIRE_TOKEN(TokenType::T_IDENTIFIER, "a", 1);
    REQUIRE_TOKEN(TokenType::T_IDENTIFIER, "b", 20);
    REQUIRE_TOKEN(TokenType::T_INTEGER_LITERAL, "1", 22);
    REQUIRE_TOKEN(TokenType::T_DIV, "/", 24);
    REQUIRE_TOKEN(TokenType::T_SEMICOLON, ";", 25);
    REQUIRE_TOKEN(TokenType::T_IDENTIFIER, "c", 33);
    REQUIRE_ERRORS(1);
    REQUIRE_ERROR(ParseErrorType::E_INVALID_CHARACTERS, "_", 0);
)

TEST_LEXER("/**/", REQUIRE_NO_ERRORS();)
TEST_LEXER("//", REQUIRE_NO_ERRORS();)
TEST_LEXER("//\n", REQUIRE_NO_ERRORS();)
TEST_LEXER("///*", REQUIRE_NO_ERRORS();)
TEST_LEXER("///**/", REQUIRE_NO_ERRORS();)
TEST_LEXER("/*//*/", REQUIRE_NO_ERRORS();)
TEST_LEXER("/* 1 // 2 */", REQUIRE_NO_ERRORS();)
TEST_LEXER("/* \r\n \n \r */", REQUIRE_NO_ERRORS();)
TEST_LEXER("\n/* \n */\n/* \n */\n", REQUIRE_NO_ERRORS();)

TEST_LEXER("_/*_*/_//_\n_", \
    REQUIRE_ERRORS(1);
    REQUIRE_ERROR(ParseErrorType::E_INVALID_CHARACTERS, "___", 0);
)

TEST_LEXER("a_/*_*/_//_\n_", \
    REQUIRE_TOKEN(TokenType::T_IDENTIFIER, "a_", 0);
    REQUIRE_ERRORS(1);
    REQUIRE_ERROR(ParseErrorType::E_INVALID_CHARACTERS, "__", 7);
)

TEST_LEXER("a _/*_*/b_//_\n_ c", \
    REQUIRE_TOKEN(TokenType::T_IDENTIFIER, "a", 0);
    REQUIRE_TOKEN(TokenType::T_IDENTIFIER, "b_", 8);
    REQUIRE_TOKEN(TokenType::T_IDENTIFIER, "c", 16);
    REQUIRE_ERRORS(2);
    REQUIRE_ERROR(ParseErrorType::E_INVALID_CHARACTERS, "_", 2);
    REQUIRE_ERROR(ParseErrorType::E_INVALID_CHARACTERS, "_", 14);
)

TEST_LEXER("1 /* 2 */ 3 /* 4 */ 5", \
    REQUIRE_TOKEN(TokenType::T_INTEGER_LITERAL, "1", 0);
    REQUIRE_TOKEN(TokenType::T_INTEGER_LITERAL, "3", 10);
    REQUIRE_TOKEN(TokenType::T_INTEGER_LITERAL, "5", 20);
    REQUIRE_NO_ERRORS();
)

TEST_LEXER("1 // 2 \n 3 // 4 \n 5", \
    REQUIRE_TOKEN(TokenType::T_INTEGER_LITERAL, "1", 0);
    REQUIRE_TOKEN(TokenType::T_INTEGER_LITERAL, "3", 9);
    REQUIRE_TOKEN(TokenType::T_INTEGER_LITERAL, "5", 18);
    REQUIRE_NO_ERRORS();
)

TEST_LEXER("/*", \
    REQUIRE_ERRORS(1);
    REQUIRE_ERROR(ParseErrorType::E_UNTERMINATED_COMMENT, "/*", 0);
)

TEST_LEXER("a /*", \
    REQUIRE_TOKEN(TokenType::T_IDENTIFIER, "a", 0);
    REQUIRE_ERRORS(1);
    REQUIRE_ERROR(ParseErrorType::E_UNTERMINATED_COMMENT, "/*", 2);
)

TEST_LEXER("a /* a b c 1 2 3 \n // /*", \
    REQUIRE_TOKEN(TokenType::T_IDENTIFIER, "a", 0);
    REQUIRE_ERRORS(1);
    REQUIRE_ERROR(ParseErrorType::E_UNTERMINATED_COMMENT, "/*", 2);
)