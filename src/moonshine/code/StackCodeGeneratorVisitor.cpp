#include "moonshine/code/StackCodeGeneratorVisitor.h"

#include "moonshine/lexer/TokenType.h"
#include "moonshine/semantic/Type.h"
#include "moonshine/semantic/SymbolTable.h"

#include <utility>
#include <iomanip>

namespace moonshine { namespace code {

using std::endl;

StackCodeGeneratorVisitor::StackCodeGeneratorVisitor(std::ostream& dataStream, std::ostream& textStream)
    : dataStream_(dataStream), textStream_(textStream)
{
    for (int i = 12; i >= 1; --i) {
        registers_.emplace("r" + std::to_string(i));
    }
}

void StackCodeGeneratorVisitor::visit(ast::prog* node)
{
    Visitor::visit(node);

    data() << "% buffer space used for console output" << endl;
    data("buf") << "res 20" << endl;
}

void StackCodeGeneratorVisitor::visit(ast::statBlock* node)
{
    // only execute this visitor for the main program function statBlock
    if (!dynamic_cast<ast::prog*>(node->parent())) {
        Visitor::visit(node);
        return;
    }

    text() << "entry" << endl;
    addi(SP, ZR, "topaddr");

    text() << "%% program body begin" << endl;
    Visitor::visit(node);
    text() << "%% program body end" << endl;

    text() << "hlt" << endl;
}

void StackCodeGeneratorVisitor::visit(ast::num* node)
{
    Visitor::visit(node);

    if (!node->symbolTableEntry()) {
        return;
    }

    // TODO: float
    if (node->type()->type != semantic::Type::INT) {
        return;
    }

    text() << "% num: " << node->symbolTableEntry()->name() << " := " << node->token()->value << endl;

    auto r1 = reg();

    addi(r1, ZR, node->token()->value);
    sw(-node->symbolTableEntry()->offset(), SP, r1);

    regPush(r1);
}

void StackCodeGeneratorVisitor::visit(ast::var* node)
{
    Visitor::visit(node);

    auto table = node->closestSymbolTable();

    node->symbolTableEntry() = std::make_shared<semantic::SymbolTableEntry>();
    node->symbolTableEntry()->setName(node->child()->symbolTableEntry()->name());
    node->symbolTableEntry()->setKind(node->child()->symbolTableEntry()->kind());
    //node->symbolTableEntry()->setType(std::unique_ptr<SymbolType>(new SymbolType(*node->child()->symbolTableEntry()->type())));
    node->symbolTableEntry()->setSize(node->child()->symbolTableEntry()->size());

    // TODO
    if (dynamic_cast<ast::dataMember*>(node->child())) {
        node->symbolTableEntry()->setOffset(node->child()->relativeOffset);
    } else {
        node->symbolTableEntry()->setOffset(node->child()->symbolTableEntry()->offset());
    }

    //text() << "% var: " << node->symbolTableEntry()->name() << endl;
    //
    //auto r1 = reg();
    //
    //lw(r1, -(*table)[dynamic_cast<ast::id*>(node->child(0)->child(0))->token()->value]->offset(), SP);
    //sw(-node->symbolTableEntry()->offset(), SP, r1);
    //
    //regPush(r1);
}

void StackCodeGeneratorVisitor::visit(ast::addOp* node)
{
    Visitor::visit(node);

    auto r1 = reg();
    auto r2 = reg();
    auto r3 = reg();

    text() << "% addOp: " << node->symbolTableEntry()->name() << " := "
           << node->child(0)->symbolTableEntry()->name() << ' ' << node->token()->value << ' ' << node->child(1)->symbolTableEntry()->name() << endl;

    lw(r1, -node->child(0)->symbolTableEntry()->offset(), SP);
    lw(r2, -node->child(1)->symbolTableEntry()->offset(), SP);

    switch (node->token()->type) {
        case TokenType::T_PLUS:
            add(r3, r1, r2);
            break;
        case TokenType::T_MINUS:
            sub(r3, r1, r2);
            break;
        case TokenType::T_OR:
            // TODO
            break;
        default:
            break;
    }

    sw(-node->symbolTableEntry()->offset(), SP, r3);

    regPush(r3);
    regPush(r2);
    regPush(r1);
}

void StackCodeGeneratorVisitor::visit(ast::multOp* node)
{
    Visitor::visit(node);

    text() << "% multOp: " << node->symbolTableEntry()->name() << " := "
           << node->child(0)->symbolTableEntry()->name() << ' ' << node->token()->value << ' ' << node->child(1)->symbolTableEntry()->name() << endl;

    auto r1 = reg();
    auto r2 = reg();
    auto r3 = reg();

    // load data from lhs and rhs
    lw(r1, -node->child(0)->symbolTableEntry()->offset(), SP);
    lw(r2, -node->child(1)->symbolTableEntry()->offset(), SP);

    switch (node->token()->type) {
        case TokenType::T_MUL:
            mul(r3, r1, r2);
            break;
        case TokenType::T_DIV:
            div(r3, r1, r2);
            break;
        case TokenType::T_AND:
            // TODO
            break;
        default:
            break;
    }

    sw(-node->symbolTableEntry()->offset(), SP, r3);

    regPush(r3);
    regPush(r2);
    regPush(r1);
}

void StackCodeGeneratorVisitor::visit(ast::relOp* node)
{
    Visitor::visit(node);

    text() << "% relOp: " << node->symbolTableEntry()->name() << " := "
           << node->child(0)->symbolTableEntry()->name() << ' ' << node->token()->value << ' ' << node->child(1)->symbolTableEntry()->name() << endl;

    auto r1 = reg();
    auto r2 = reg();
    auto r3 = reg();

    // load data from lhs and rhs
    lw(r1, -node->child(0)->symbolTableEntry()->offset(), SP);
    lw(r2, -node->child(1)->symbolTableEntry()->offset(), SP);

    // add and store
    switch (node->token()->type) {
        case TokenType::T_IS_SMALLER:
            clt(r3, r1, r2);
            break;
        case TokenType::T_IS_SMALLER_OR_EQUAL:
            cle(r3, r1, r2);
            break;
        case TokenType::T_IS_GREATER:
            cgt(r3, r1, r2);
            break;
        case TokenType::T_IS_GREATER_OR_EQUAL:
            cge(r3, r1, r2);
            break;
        case TokenType::T_IS_EQUAL:
            ceq(r3, r1, r2);
            break;
        case TokenType::T_IS_NOT_EQUAL:
            cne(r3, r1, r2);
            break;
        default:
            break;
    }

    sw(-node->symbolTableEntry()->offset(), SP, r3);

    regPush(r3);
    regPush(r2);
    regPush(r1);
}

void StackCodeGeneratorVisitor::visit(ast::assignStat* node)
{
    Visitor::visit(node);

    text() << "% assignStat: " << node->child(0)->symbolTableEntry()->name() << " := " <<  node->child(1)->symbolTableEntry()->name() << endl;

    auto r1 = reg();

    lw(r1, -node->child(1)->symbolTableEntry()->offset(), SP);
    sw(-node->child(0)->symbolTableEntry()->offset(), SP, r1);

    regPush(r1);
}

void StackCodeGeneratorVisitor::visit(ast::putStat* node)
{
    Visitor::visit(node);

    auto table = node->closestSymbolTable();

    text() << "% putStat: " << node->child()->symbolTableEntry()->name() << endl;

    auto r1 = reg();

    // put the value to be printed into a register
    lw(r1, -node->child()->symbolTableEntry()->offset(), SP);

    // make the stack frame pointer point to the called function's stack frame
    addi(SP, SP, -table->size()); // put value on stack

    // copy the value to be printed in the called function's stack frame
    sw(-8, SP, r1);
    addi(r1, ZR, "buf"); // link buffer to stack
    sw(-12, SP, r1);
    jl(JL, "intstr"); // convert int to string for output

    // receive the return value in r13 and right away put it in the next called function's stack frame
    sw(-8, SP, RV);
    jl(JL, "putstr");

    // make the stack frame pointer point back to the current function's stack frame
    subi(SP, SP, -table->size());

    regPush(r1);
}

void StackCodeGeneratorVisitor::visit(ast::returnStat* node)
{
    Visitor::visit(node);

    text() << "% returnStat: " << node->child()->symbolTableEntry()->name() << endl;

    auto r1 = reg();

    lw(r1, -node->child()->symbolTableEntry()->offset(), SP);
    sw(-4, SP, r1);

    regPush(r1);
}

void StackCodeGeneratorVisitor::visit(ast::funcDef* node)
{
    auto function = node->symbolTableEntry();

    // create the tag to jump onto
    // and copy the jumping-back address value in the called function's stack frame
    text(node->symbolTableEntry()->name()) << "% funcDef: " << function->name() << endl;
    sw(-8, SP, JL);

    // generate the code for the function body
    text() << "%% function body begin" << endl;
    Visitor::visit(node);
    text() << "%% function body end" << endl;

    // copy back the jumping-back address into r15
    lw(JL, -8, SP);

    // jump back to the calling function
    jr(JL);
}

void StackCodeGeneratorVisitor::visit(ast::dataMember* node)
{
    Visitor::visit(node);

    auto table = node->closestSymbolTable().get();
    auto entry = node->symbolTableEntry();
    int offset = entry->offset();
    while (table && table != entry->parentTable()) {
        table = table->parentEntry()->parentTable();
        offset -= table->size();
    }

    node->relativeOffset = offset;
}

void StackCodeGeneratorVisitor::visit(ast::fCall* node)
{
    Visitor::visit(node);

    auto table = node->closestSymbolTable();
    auto idNode = dynamic_cast<ast::id*>(node->child(0));
    auto function = (*table)[idNode->token()->value];

    text() << "% fCall: " << function->name() << endl;

    auto r1 = reg();

    auto currentParam = function->parameters().begin();

    for (auto aParam = node->child(1)->child(); aParam != nullptr; aParam = aParam->next()) {
        text() << "%% fparam begin: " << (*currentParam)->name() << endl;

        lw(r1, -aParam->symbolTableEntry()->offset(), SP);
        sw(-table->size() - (*currentParam)->offset(), SP, r1);

        text() << "% fparam end: " << (*currentParam)->name() << endl;
        currentParam++;
    }

    // make the stack frame pointer point to the called function's stack frame
    addi(SP, SP, -table->size());

    // jump to the called function's code
    // here the function's name is the label
    // TODO: unique names? should we use label() for function names? perhaps create in funcDef and store in entry
    jl(JL, function->name());

    // upon jumping back, set the stack frame pointer back to the current function's stack frame
    subi(SP, SP, -table->size());

    // copy the return value in memory space to store it on the current stack frame
    // to evaluate the expression in which it is
    lw(r1, -table->size() - 4, SP);
    sw(-node->symbolTableEntry()->offset(), SP, r1);

    regPush(r1);
}

void StackCodeGeneratorVisitor::visit(ast::ifStat* node)
{
    text() << "% ifStat: begin" << endl;

    auto elseLabel = label("else");
    auto endifLabel = label("endif");

    // condition
    node->child(0)->accept(this);

    auto r1 = reg();
    lw(r1, -node->child(0)->symbolTableEntry()->offset(), SP);
    bz(r1, elseLabel);
    regPush(r1);

    // then
    node->child(1)->accept(this);

    j(endifLabel);

    // else
    text(elseLabel) << "% ifStat: else" << endl;
    node->child(2)->accept(this);

    text(endifLabel) << "% ifStat: end" << endl;
}

void StackCodeGeneratorVisitor::visit(ast::forStat* node)
{
    text() << "% forStat: begin" << endl;

    auto table = node->closestSymbolTable();
    auto parentTable = node->parent()->closestSymbolTable();
    auto forcondLabel = label("forcond");
    auto endforLabel = label("endfor");

    // move stack frame pointer into for scope
    addi(SP, SP, -parentTable->size());

    // initialization
    node->child(2)->accept(this);
    text() << "% forStat: " << dynamic_cast<ast::id*>(node->child(1))->token()->value << " := " <<  node->child(2)->symbolTableEntry()->name() << endl;
    auto r1 = reg();
    lw(r1, -node->child(2)->symbolTableEntry()->offset(), SP);
    sw(-(*table)[dynamic_cast<ast::id*>(node->child(1))->token()->value]->offset(), SP, r1);
    regPush(r1);

    // condition
    text(forcondLabel) << "% forStat: condition" << endl;
    node->child(3)->accept(this);
    r1 = reg();
    lw(r1, -node->child(3)->symbolTableEntry()->offset(), SP);
    bz(r1, endforLabel);
    regPush(r1);

    // body
    text() << "% forStat: body" << endl;

    // make the stack frame pointer point to the called function's stack frame
    node->child(5)->accept(this);

    // post
    text() << "% forStat: post" << endl;
    node->child(4)->accept(this);

    j(forcondLabel);

    // reset stack frame pointer
    text(endforLabel) << "% forStat: end" << endl;
    subi(SP, SP, -parentTable->size());
}

std::string StackCodeGeneratorVisitor::reg()
{
    auto r = registers_.top();
    registers_.pop();
    return r;
}

void StackCodeGeneratorVisitor::regPush(const std::string& reg)
{
    registers_.emplace(reg);
}

void StackCodeGeneratorVisitor::add(const std::string& dest, const std::string& source, const std::string& offset)
{
    text() << "add " << dest << ',' << source << ',' << offset << endl;
}

void StackCodeGeneratorVisitor::addi(const std::string& dest, const std::string& source, const std::string& immediate)
{
    text() << "addi " << dest << ',' << source << ',' << immediate << endl;
}

void StackCodeGeneratorVisitor::addi(const std::string& dest, const std::string& source, const int& immediate)
{
    addi(dest, source, std::to_string(immediate));
}

void StackCodeGeneratorVisitor::sub(const std::string& dest, const std::string& source, const std::string& offset)
{
    text() << "sub " << dest << ',' << source << ',' << offset << endl;
}

void StackCodeGeneratorVisitor::subi(const std::string& dest, const std::string& source, const std::string& immediate)
{
    text() << "subi " << dest << ',' << source << ',' << immediate << endl;
}

void StackCodeGeneratorVisitor::subi(const std::string& dest, const std::string& source, const int& immediate)
{
    subi(dest, source, std::to_string(immediate));
}

void StackCodeGeneratorVisitor::mul(const std::string& dest, const std::string& source, const std::string& offset)
{
    text() << "mul " << dest << ',' << source << ',' << offset << endl;
}

void StackCodeGeneratorVisitor::div(const std::string& dest, const std::string& source, const std::string& offset)
{
    text() << "div " << dest << ',' << source << ',' << offset << endl;
}

void StackCodeGeneratorVisitor::lw(const std::string& dest, const int& offset, const std::string& source)
{
    text() << "lw " << dest << ',' << offset << '(' << source << ')' << endl;
}

void StackCodeGeneratorVisitor::sw(const int& offset, const std::string& dest, const std::string& source)
{
    text() << "sw " << offset << '(' << dest << ")," << source << endl;
}

void StackCodeGeneratorVisitor::jl(const std::string& store, const std::string& jmp)
{
    text() << "jl " << store << ',' << jmp << endl;
}

void StackCodeGeneratorVisitor::jr(const std::string& jmp)
{
    text() << "jr " << jmp << endl;
}

void StackCodeGeneratorVisitor::bz(const std::string& test, const std::string& jmp)
{
    text() << "bz " << test << ',' << jmp << endl;
}

void StackCodeGeneratorVisitor::bnz(const std::string& test, const std::string& jmp)
{
    text() << "bnz " << test << ',' << jmp << endl;
}

void StackCodeGeneratorVisitor::j(const std::string& jmp)
{
    text() << "j " << jmp << endl;
}

void StackCodeGeneratorVisitor::clt(const std::string& dest, const std::string& op1, const std::string& op2)
{
    text() << "clt " << dest << ',' << op1 << ',' << op2 << endl;
}

void StackCodeGeneratorVisitor::cle(const std::string& dest, const std::string& op1, const std::string& op2)
{
    text() << "cle " << dest << ',' << op1 << ',' << op2 << endl;
}

void StackCodeGeneratorVisitor::cgt(const std::string& dest, const std::string& op1, const std::string& op2)
{
    text() << "cgt " << dest << ',' << op1 << ',' << op2 << endl;
}

void StackCodeGeneratorVisitor::cge(const std::string& dest, const std::string& op1, const std::string& op2)
{
    text() << "cge " << dest << ',' << op1 << ',' << op2 << endl;
}


void StackCodeGeneratorVisitor::ceq(const std::string& dest, const std::string& op1, const std::string& op2)
{
    text() << "ceq " << dest << ',' << op1 << ',' << op2 << endl;
}

void StackCodeGeneratorVisitor::cne(const std::string& dest, const std::string& op1, const std::string& op2)
{
    text() << "cne " << dest << ',' << op1 << ',' << op2 << endl;
}

std::ostream& StackCodeGeneratorVisitor::data()
{
    return dataStream_ << indent_;
}

std::ostream& StackCodeGeneratorVisitor::data(const std::string& label)
{
    return dataStream_ << std::setw(10) << std::left << label;
}

std::ostream& StackCodeGeneratorVisitor::text()
{
    return textStream_ << indent_;
}

std::ostream& StackCodeGeneratorVisitor::text(const std::string& label)
{
    return textStream_ << std::left << std::setw(10) << label;
}

std::string StackCodeGeneratorVisitor::label(const std::string& label)
{
    return label + std::to_string(labels_[label]++);
}

}}
