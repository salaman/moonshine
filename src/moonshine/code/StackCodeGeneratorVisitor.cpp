#include "moonshine/code/StackCodeGeneratorVisitor.h"

#include "moonshine/lexer/TokenType.h"
#include "moonshine/semantic/Type.h"

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

    add(r3, r1, r2);

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

    // add and store
    mul(r3, r1, r2);
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
    // create the tag to jump onto
    // and copy the jumping-back address value in the called function's stack frame
    //m_moonExecCode += String.format("%-10s",p_node.getData())  + "sw -4(r14),r15\n" ;
    text(node->symbolTableEntry()->name()) << "% funcDef: " << node->symbolTableEntry()->name() << endl;
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
}

void StackCodeGeneratorVisitor::visit(ast::fCall* node)
{
    Visitor::visit(node);

    auto table = node->closestSymbolTable();
    auto idNode = dynamic_cast<ast::id*>(node->child(0));

    text() << "% fCall: " << idNode->token()->value << endl;

    auto r1 = reg();

    // make the stack frame pointer point to the called function's stack frame
    addi(SP, SP, -table->size());

    // jump to the called function's code
    // here the function's name is the label
    // a unique label generator is necessary in the general case
    jl(JL, idNode->token()->value);

    // upon jumping back, set the stack frame pointer back to the current function's stack frame
    subi(SP, SP, -table->size());

    // copy the return value in memory space to store it on the current stack frame
    // to evaluate the expression in which it is
    lw(r1, -table->size() - 4, SP);
    sw(-node->symbolTableEntry()->offset(), SP, r1);

    regPush(r1);
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
    text() << "add " << dest << ',' << source << ',' << offset << std::endl;
}

void StackCodeGeneratorVisitor::addi(const std::string& dest, const std::string& source, const std::string& immediate)
{
    text() << "addi " << dest << ',' << source << ',' << immediate << std::endl;
}

void StackCodeGeneratorVisitor::addi(const std::string& dest, const std::string& source, const int& immediate)
{
    addi(dest, source, std::to_string(immediate));
}

void StackCodeGeneratorVisitor::mul(const std::string& dest, const std::string& source, const std::string& offset)
{
    text() << "mul " << dest << ',' << source << ',' << offset << std::endl;
}

void StackCodeGeneratorVisitor::subi(const std::string& dest, const std::string& source, const std::string& immediate)
{
    text() << "subi " << dest << ',' << source << ',' << immediate << std::endl;
}

void StackCodeGeneratorVisitor::subi(const std::string& dest, const std::string& source, const int& immediate)
{
    subi(dest, source, std::to_string(immediate));
}

void StackCodeGeneratorVisitor::lw(const std::string& dest, const int& offset, const std::string& source)
{
    text() << "lw " << dest << ',' << offset << '(' << source << ')' << std::endl;
}

void StackCodeGeneratorVisitor::sw(const int& offset, const std::string& dest, const std::string& source)
{
    text() << "sw " << offset << '(' << dest << ")," << source << std::endl;
}

void StackCodeGeneratorVisitor::jl(const std::string& store, const std::string& jmp)
{
    text() << "jl " << store << ',' << jmp << std::endl;
}

void StackCodeGeneratorVisitor::jr(const std::string& jmp)
{
    text() << "jr " << jmp << std::endl;
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

}}
