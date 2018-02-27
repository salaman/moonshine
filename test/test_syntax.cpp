#include <catch/catch.hpp>

#include <moonshine/lexer/Lexer.h>
#include <moonshine/syntax/Parser.h>

#include <sstream>
#include <memory>

using namespace moonshine;

#define TEST_AST(INPUT, SEM) \
TEST_CASE(INPUT, "[syntax]") { \
    Lexer lex; \
    syntax::Grammar grammar("grammar.txt",  "table.json", "first.txt", "follow.txt"); \
    \
    std::istringstream stream((INPUT)); \
    lex.startLexing(&stream); \
    syntax::Parser parser(grammar); \
    \
    std::unique_ptr<ast::Node> astRoot; \
    astRoot = parser.parse(&lex, nullptr); \
    \
    REQUIRE(astRoot != nullptr); \
    std::ostringstream oss; \
    astRoot->print(&oss); \
    REQUIRE(oss.str() == (SEM)); \
}

TEST_AST(
    "program { };",
    "prog{classList funcDefList statBlock}")

TEST_AST(
    "program { int a; };",
    "prog{classList funcDefList statBlock{varDecl{type(int) id(a) dimList}}}")

TEST_AST(
    "program { float a; };",
    "prog{classList funcDefList statBlock{varDecl{type(float) id(a) dimList}}}")

TEST_AST(
    "program { ident a; };",
    "prog{classList funcDefList statBlock{varDecl{id(ident) id(a) dimList}}}")

TEST_AST(
    "program { int a[1]; };",
    "prog{classList funcDefList statBlock{varDecl{type(int) id(a) dimList{num(1)}}}}")

TEST_AST(
    "program { int a[1][2]; };",
    "prog{classList funcDefList statBlock{varDecl{type(int) id(a) dimList{num(1) num(2)}}}}")

TEST_AST(
    "program { id a[1][2][3]; };",
    "prog{classList funcDefList statBlock{varDecl{id(id) id(a) dimList{num(1) num(2) num(3)}}}}")

TEST_AST(
    "program { a = 1; };",
    "prog{classList funcDefList statBlock{assignStat{var{dataMember{id(a) indexList}} num(1)}}}")

TEST_AST(
    "program { a = 1 + 2; };",
    "prog{classList funcDefList statBlock{assignStat{var{dataMember{id(a) indexList}} addOp{num(1) num(2)}(+)}}}")

TEST_AST(
    "program { a = b + c; };",
    "prog{classList funcDefList statBlock{assignStat{var{dataMember{id(a) indexList}} addOp{var{dataMember{id(b) indexList}} var{dataMember{id(c) indexList}}}(+)}}}")

TEST_AST(
    "program { a = 1 + b; };",
    "prog{classList funcDefList statBlock{assignStat{var{dataMember{id(a) indexList}} addOp{num(1) var{dataMember{id(b) indexList}}}(+)}}}")

TEST_AST(
    "program { value = -1; };",
    "prog{classList funcDefList statBlock{assignStat{var{dataMember{id(value) indexList}} sign{num(1)}(-)}}}")

TEST_AST(
    "program { value = -a; };",
    "prog{classList funcDefList statBlock{assignStat{var{dataMember{id(value) indexList}} sign{var{dataMember{id(a) indexList}}}(-)}}}")

TEST_AST(
    "program { a = 1 + 2 * 3; };",
    "prog{classList funcDefList statBlock{assignStat{var{dataMember{id(a) indexList}} addOp{num(1) multOp{num(2) num(3)}(*)}(+)}}}")

TEST_AST(
    "program { a = (1 + 2) * 3; };",
    "prog{classList funcDefList statBlock{assignStat{var{dataMember{id(a) indexList}} multOp{addOp{num(1) num(2)}(+) num(3)}(*)}}}")

TEST_AST(
    "program { a = 1 + 2 and 3; };",
    "prog{classList funcDefList statBlock{assignStat{var{dataMember{id(a) indexList}} addOp{num(1) multOp{num(2) num(3)}(and)}(+)}}}")

TEST_AST(
    "program { a = 1 > 2 and 3; };",
    "prog{classList funcDefList statBlock{assignStat{var{dataMember{id(a) indexList}} relOp{num(1) multOp{num(2) num(3)}(and)}(>)}}}")

TEST_AST(
    "program { a = 1 or 2 >= 3; };",
    "prog{classList funcDefList statBlock{assignStat{var{dataMember{id(a) indexList}} relOp{addOp{num(1) num(2)}(or) num(3)}(>=)}}}")

TEST_AST(
    "program { value = 1.05 + ((2.04 * 2.47) - 3.0) + 7.0006 > 1 and not - 1; };",
    "prog{classList funcDefList statBlock{assignStat{var{dataMember{id(value) indexList}} relOp{addOp{addOp{num(1.05) addOp{multOp{num(2.04) num(2.47)}(*) num(3.0)}(-)}(+) num(7.0006)}(+) multOp{num(1) notFactor{sign{num(1)}(-)}}(and)}(>)}}}")

TEST_AST(
    "program { int idx[1]; };",
    "prog{classList funcDefList statBlock{varDecl{type(int) id(idx) dimList{num(1)}}}}")

TEST_AST(
    "program { id idx[1][3]; };",
    "prog{classList funcDefList statBlock{varDecl{id(id) id(idx) dimList{num(1) num(3)}}}}")

TEST_AST(
    "program { float idx[1][2][3]; };",
    "prog{classList funcDefList statBlock{varDecl{type(float) id(idx) dimList{num(1) num(2) num(3)}}}}")

TEST_AST(
    "program { a(b).c = d().e[f][g].h(); };",
    "prog{classList funcDefList statBlock{assignStat{var{fCall{id(a) aParams{var{dataMember{id(b) indexList}}}} dataMember{id(c) indexList}} var{fCall{id(d) aParams} dataMember{id(e) indexList{var{dataMember{id(f) indexList}} var{dataMember{id(g) indexList}}}} fCall{id(h) aParams}}}}}")

TEST_AST(
    "program { a().c = c; };",
    "prog{classList funcDefList statBlock{assignStat{var{fCall{id(a) aParams} dataMember{id(c) indexList}} var{dataMember{id(c) indexList}}}}}")

TEST_AST(
    "program { a(1+2).b[3].c = c(4*5).d[6]; };",
    "prog{classList funcDefList statBlock{assignStat{var{fCall{id(a) aParams{addOp{num(1) num(2)}(+)}} dataMember{id(b) indexList{num(3)}} dataMember{id(c) indexList}} var{fCall{id(c) aParams{multOp{num(4) num(5)}(*)}} dataMember{id(d) indexList{num(6)}}}}}}")

TEST_AST(
    "program { arrayUtility[utility.var1[1][2][3][4][5][6][idx+maxValue]][1][1][1].var2 = 2.5; };",
    "prog{classList funcDefList statBlock{assignStat{var{dataMember{id(arrayUtility) indexList{var{dataMember{id(utility) indexList} dataMember{id(var1) indexList{num(1) num(2) num(3) num(4) num(5) num(6) addOp{var{dataMember{id(idx) indexList}} var{dataMember{id(maxValue) indexList}}}(+)}}} num(1) num(1) num(1)}} dataMember{id(var2) indexList}} num(2.5)}}}")

TEST_AST(
    "program { if (id) then {} else {}; };",
    "prog{classList funcDefList statBlock{ifStat{var{dataMember{id(id) indexList}} statBlock statBlock}}}")

TEST_AST(
    "program { if (id) then else; };",
    "prog{classList funcDefList statBlock{ifStat{var{dataMember{id(id) indexList}} statBlock statBlock}}}")

TEST_AST(
    "program { if (id > 1) then {} else {}; };",
    "prog{classList funcDefList statBlock{ifStat{relOp{var{dataMember{id(id) indexList}} num(1)}(>) statBlock statBlock}}}")

TEST_AST(
    "program { if (a and b) then {} else {}; };",
    "prog{classList funcDefList statBlock{ifStat{multOp{var{dataMember{id(a) indexList}} var{dataMember{id(b) indexList}}}(and) statBlock statBlock}}}")

TEST_AST(
    "program { if (a) then { b = c; } else {}; };",
    "prog{classList funcDefList statBlock{ifStat{var{dataMember{id(a) indexList}} statBlock{assignStat{var{dataMember{id(b) indexList}} var{dataMember{id(c) indexList}}}} statBlock}}}")

TEST_AST(
    "program { if (a) then { b = c; } else; };",
    "prog{classList funcDefList statBlock{ifStat{var{dataMember{id(a) indexList}} statBlock{assignStat{var{dataMember{id(b) indexList}} var{dataMember{id(c) indexList}}}} statBlock}}}")

TEST_AST(
    "program { if (a) then {} else { b = c; }; };",
    "prog{classList funcDefList statBlock{ifStat{var{dataMember{id(a) indexList}} statBlock statBlock{assignStat{var{dataMember{id(b) indexList}} var{dataMember{id(c) indexList}}}}}}}")

TEST_AST(
    "program { if (a) then else { b = c; }; };",
    "prog{classList funcDefList statBlock{ifStat{var{dataMember{id(a) indexList}} statBlock statBlock{assignStat{var{dataMember{id(b) indexList}} var{dataMember{id(c) indexList}}}}}}}")

TEST_AST(
    "program { if (a) then { b = c; } else { d = e; }; };",
    "prog{classList funcDefList statBlock{ifStat{var{dataMember{id(a) indexList}} statBlock{assignStat{var{dataMember{id(b) indexList}} var{dataMember{id(c) indexList}}}} statBlock{assignStat{var{dataMember{id(d) indexList}} var{dataMember{id(e) indexList}}}}}}}")

TEST_AST(
    "program { for(int idx = 1; idx <= 99; idx = idx + 1) {}; };",
    "prog{classList funcDefList statBlock{forStat{type(int) id(idx) num(1) relOp{var{dataMember{id(idx) indexList}} num(99)}(<=) assignStat{var{dataMember{id(idx) indexList}} addOp{var{dataMember{id(idx) indexList}} num(1)}(+)} statBlock}}}")

TEST_AST(
    "program { for(int idx = 1; idx <= 99; idx = idx + 1); };",
    "prog{classList funcDefList statBlock{forStat{type(int) id(idx) num(1) relOp{var{dataMember{id(idx) indexList}} num(99)}(<=) assignStat{var{dataMember{id(idx) indexList}} addOp{var{dataMember{id(idx) indexList}} num(1)}(+)} statBlock}}}")

TEST_AST(
    "program { for(int idx = 1; idx <= 99; idx = idx + 1) { a = b; }; };",
    "prog{classList funcDefList statBlock{forStat{type(int) id(idx) num(1) relOp{var{dataMember{id(idx) indexList}} num(99)}(<=) assignStat{var{dataMember{id(idx) indexList}} addOp{var{dataMember{id(idx) indexList}} num(1)}(+)} statBlock{assignStat{var{dataMember{id(a) indexList}} var{dataMember{id(b) indexList}}}}}}}")

TEST_AST(
    "program { get (id); };",
    "prog{classList funcDefList statBlock{getStat{var{dataMember{id(id) indexList}}}}}")

TEST_AST(
    "program { get (id[a].b); };",
    "prog{classList funcDefList statBlock{getStat{var{dataMember{id(id) indexList{var{dataMember{id(a) indexList}}}} dataMember{id(b) indexList}}}}}")

TEST_AST(
    "program { put (id); };",
    "prog{classList funcDefList statBlock{putStat{var{dataMember{id(id) indexList}}}}}")

TEST_AST(
    "program { put (id[a].b); };",
    "prog{classList funcDefList statBlock{putStat{var{dataMember{id(id) indexList{var{dataMember{id(a) indexList}}}} dataMember{id(b) indexList}}}}}")

TEST_AST(
    "program { return (id); };",
    "prog{classList funcDefList statBlock{returnStat{var{dataMember{id(id) indexList}}}}}")

TEST_AST(
    "program { return (id[a].b); };",
    "prog{classList funcDefList statBlock{returnStat{var{dataMember{id(id) indexList{var{dataMember{id(a) indexList}}}} dataMember{id(b) indexList}}}}}")

TEST_AST(
    "program { if (a) then { if (b) then {} else { if (c) then {} else { return (0); }; }; } else {}; };",
    "prog{classList funcDefList statBlock{ifStat{var{dataMember{id(a) indexList}} statBlock{ifStat{var{dataMember{id(b) indexList}} statBlock statBlock{ifStat{var{dataMember{id(c) indexList}} statBlock statBlock{returnStat{num(0)}}}}}} statBlock}}}")

TEST_AST(
    "program { if (a) then { return (id); } else { for(int idx = 1; idx <= 99; idx = idx + 1) { a = b; }; }; };",
    "prog{classList funcDefList statBlock{ifStat{var{dataMember{id(a) indexList}} statBlock{returnStat{var{dataMember{id(id) indexList}}}} statBlock{forStat{type(int) id(idx) num(1) relOp{var{dataMember{id(idx) indexList}} num(99)}(<=) assignStat{var{dataMember{id(idx) indexList}} addOp{var{dataMember{id(idx) indexList}} num(1)}(+)} statBlock{assignStat{var{dataMember{id(a) indexList}} var{dataMember{id(b) indexList}}}}}}}}}")

TEST_AST(
    "program { x = func(); };",
    "prog{classList funcDefList statBlock{assignStat{var{dataMember{id(x) indexList}} var{fCall{id(func) aParams}}}}}")

TEST_AST(
    "program { x = func(i*j); };",
    "prog{classList funcDefList statBlock{assignStat{var{dataMember{id(x) indexList}} var{fCall{id(func) aParams{multOp{var{dataMember{id(i) indexList}} var{dataMember{id(j) indexList}}}(*)}}}}}}")

TEST_AST(
    "program { x = a.b[1].c(); };",
    "prog{classList funcDefList statBlock{assignStat{var{dataMember{id(x) indexList}} var{dataMember{id(a) indexList} dataMember{id(b) indexList{num(1)}} fCall{id(c) aParams}}}}}")

TEST_AST(
    "program { x = a.b[1].c(i*j); };",
    "prog{classList funcDefList statBlock{assignStat{var{dataMember{id(x) indexList}} var{dataMember{id(a) indexList} dataMember{id(b) indexList{num(1)}} fCall{id(c) aParams{multOp{var{dataMember{id(i) indexList}} var{dataMember{id(j) indexList}}}(*)}}}}}}")

TEST_AST(
    "class Utility {}; program {};",
    "prog{classList{classDecl{id(Utility) inherList membList}} funcDefList statBlock}")

TEST_AST(
    "class Utility { float abc(); }; program {};",
    "prog{classList{classDecl{id(Utility) inherList membList{funcDecl{type(float) id(abc) fparamList}}}} funcDefList statBlock}")

TEST_AST(
    "class Utility { float abc(int array[100]); }; program {};",
    "prog{classList{classDecl{id(Utility) inherList membList{funcDecl{type(float) id(abc) fparamList{fparam{type(int) id(array) dimList{num(100)}}}}}}} funcDefList statBlock}")

TEST_AST(
    "class Utility : A {}; program {};",
    "prog{classList{classDecl{id(Utility) inherList{id(A)} membList}} funcDefList statBlock}")

TEST_AST(
    "class Utility : A, B, C { float abc(); }; program {};",
    "prog{classList{classDecl{id(Utility) inherList{id(A) id(B) id(C)} membList{funcDecl{type(float) id(abc) fparamList}}}} funcDefList statBlock}")

TEST_AST(
    "class Utility : A { int member1; }; program {};",
    "prog{classList{classDecl{id(Utility) inherList{id(A)} membList{varDecl{type(int) id(member1) dimList}}}} funcDefList statBlock}")

TEST_AST(
    "class Utility : A { int member1; int member2; }; program {};",
    "prog{classList{classDecl{id(Utility) inherList{id(A)} membList{varDecl{type(int) id(member1) dimList} varDecl{type(int) id(member2) dimList}}}} funcDefList statBlock}")

TEST_AST(
    "class Utility : A { int member1; int member2; float abc(); }; program {};",
    "prog{classList{classDecl{id(Utility) inherList{id(A)} membList{varDecl{type(int) id(member1) dimList} varDecl{type(int) id(member2) dimList} funcDecl{type(float) id(abc) fparamList}}}} funcDefList statBlock}")

TEST_AST(
    "class Utility : A { int member1; float abc(); }; program {};",
    "prog{classList{classDecl{id(Utility) inherList{id(A)} membList{varDecl{type(int) id(member1) dimList} funcDecl{type(float) id(abc) fparamList}}}} funcDefList statBlock}")

TEST_AST(
    "class A {}; class B {}; program {};",
    "prog{classList{classDecl{id(A) inherList membList} classDecl{id(B) inherList membList}} funcDefList statBlock}")

TEST_AST(
    "class A : X, Y {}; class B: X, Y {}; program {};",
    "prog{classList{classDecl{id(A) inherList{id(X) id(Y)} membList} classDecl{id(B) inherList{id(X) id(Y)} membList}} funcDefList statBlock}")

TEST_AST(
    "int a() {}; program {};",
    "prog{classList funcDefList{funcDef{type(int) id(a) nul fparamList statBlock}} statBlock}")

TEST_AST(
    "int a(int array) {}; program {};",
    "prog{classList funcDefList{funcDef{type(int) id(a) nul fparamList{fparam{type(int) id(array) dimList}} statBlock}} statBlock}")

TEST_AST(
    "int a(ident var) {}; program {};",
    "prog{classList funcDefList{funcDef{type(int) id(a) nul fparamList{fparam{type(ident) id(var) dimList}} statBlock}} statBlock}")

TEST_AST(
    "int a(int array[100]) {}; program {};",
    "prog{classList funcDefList{funcDef{type(int) id(a) nul fparamList{fparam{type(int) id(array) dimList{num(100)}}} statBlock}} statBlock}")

TEST_AST(
    "int a(int array[100]) { int i; }; program {};",
    "prog{classList funcDefList{funcDef{type(int) id(a) nul fparamList{fparam{type(int) id(array) dimList{num(100)}}} statBlock{varDecl{type(int) id(i) dimList}}}} statBlock}")

TEST_AST(
    "int a() { int i; i = 1; }; program {};",
    "prog{classList funcDefList{funcDef{type(int) id(a) nul fparamList statBlock{varDecl{type(int) id(i) dimList} assignStat{var{dataMember{id(i) indexList}} num(1)}}}} statBlock}")

TEST_AST(
    "int X::a() {}; program {};",
    "prog{classList funcDefList{funcDef{type(int) id(X) id(a) fparamList statBlock}} statBlock}")

TEST_AST(
    "int X::a(Y y) {}; program {};",
    "prog{classList funcDefList{funcDef{type(int) id(X) id(a) fparamList{fparam{type(Y) id(y) dimList}} statBlock}} statBlock}")

TEST_AST(
    "int X::a() { int i; }; program {};",
    "prog{classList funcDefList{funcDef{type(int) id(X) id(a) fparamList statBlock{varDecl{type(int) id(i) dimList}}}} statBlock}")

TEST_AST(
    "int func(int a, float b, int c) {}; program {};",
    "prog{classList funcDefList{funcDef{type(int) id(func) nul fparamList{fparam{type(int) id(a) dimList} fparam{type(float) id(b) dimList} fparam{type(int) id(c) dimList}} statBlock}} statBlock}")
