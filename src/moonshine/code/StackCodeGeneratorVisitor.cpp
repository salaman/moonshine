#include "moonshine/code/StackCodeGeneratorVisitor.h"

#include "moonshine/lexer/TokenType.h"
#include "moonshine/semantic/Type.h"
#include "moonshine/semantic/SymbolTable.h"

#include <utility>
#include <iomanip>
#include <algorithm>

namespace moonshine { namespace code {

using std::endl;

StackCodeGeneratorVisitor::StackCodeGeneratorVisitor(std::ostream& dataStream, std::ostream& textStream)
    : dataStream_(dataStream), textStream_(textStream)
{
    indent_ = std::string(indentLength_, ' ');

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

void StackCodeGeneratorVisitor::visit(ast::sign* node)
{
    Visitor::visit(node);

    text() << "% sign: " << node->symbolTableEntry()->name() << " := -" << node->child()->symbolTableEntry()->name() << endl;

    auto r1 = reg();

    lw(r1, -node->child()->symbolTableEntry()->offset(), SP);

    switch (node->token()->type) {
        case TokenType::T_MINUS:
            muli(r1, r1, -1);
            break;
        default:
            // no-op
            break;
    }

    sw(-node->symbolTableEntry()->offset(), SP, r1);

    regPush(r1);
}

void StackCodeGeneratorVisitor::visit(ast::var* node)
{
    auto table = node->closestSymbolTable();

    text() << "% var begin: " << node->symbolTableEntry()->name() << endl;

    auto r1 = reg();

    // if the first child is a dataMember, we should see where to initially set our pointer to
    if (dynamic_cast<ast::dataMember*>(node->child())) {
        if (node->child()->symbolTableEntry()->parentTable() == table.get()) {
            // within our own symbol table
            // initialize offset var to current stack pointer
            text() << "% var: initial offset for own var" << endl;
            sw(-node->symbolTableEntry()->offset(), SP, SP);
        } else if (node->child()->symbolTableEntry()->parentTable()->parentEntry()->kind() == semantic::SymbolTableEntryKind::CLASS) {
            // inherited member from a class symbol table
            // initialize offset var to the top address of the dataMember's data
            text() << "% var: initial offset for member var" << endl;
            auto thisVar = (*table)["_this"];
            lw(r1, -thisVar->offset(), SP);
            sw(-node->symbolTableEntry()->offset(), SP, r1);
        } else {
            // a nested block (forStat)
            text() << "% var: initial offset for scope var" << endl;
            addi(r1, SP, node->child()->symbolTableEntry()->parentTable()->size());
            sw(-node->symbolTableEntry()->offset(), SP, r1);
        }
    }

    regPush(r1);

    // evaluate dataMembers and fCalls
    Visitor::visit(node);

    // if the last child was a dataMember, we should adjust our offset by adding the var size to it
    // since we'll be pointing at the top of the variable on the stack instead of the bottom
    if (dynamic_cast<ast::dataMember*>(node->rightmostChild())) {
        text() << "% var: adjust offset" << endl;

        r1 = reg();
        lw(r1, -node->symbolTableEntry()->offset(), SP);
        addi(r1, r1, -node->symbolTableEntry()->size());
        sw(-node->symbolTableEntry()->offset(), SP, r1);
        regPush(r1);
    }

    text() << "% var end" << endl;
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
    if (dynamic_cast<ast::var*>(node->child(0))) {
        // indirect
        lw(r1, 0, r1);
    }

    lw(r2, -node->child(1)->symbolTableEntry()->offset(), SP);
    if (dynamic_cast<ast::var*>(node->child(1))) {
        // indirect
        lw(r2, 0, r2);
    }

    switch (node->token()->type) {
        case TokenType::T_PLUS:
            add(r3, r1, r2);
            break;
        case TokenType::T_MINUS:
            sub(r3, r1, r2);
            break;
        case TokenType::T_OR:
            orOp(r3, r1, r2);
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
    if (dynamic_cast<ast::var*>(node->child(0))) {
        // indirect
        lw(r1, 0, r1);
    }

    lw(r2, -node->child(1)->symbolTableEntry()->offset(), SP);
    if (dynamic_cast<ast::var*>(node->child(1))) {
        // indirect
        lw(r2, 0, r2);
    }

    switch (node->token()->type) {
        case TokenType::T_MUL:
            mul(r3, r1, r2);
            break;
        case TokenType::T_DIV:
            div(r3, r1, r2);
            break;
        case TokenType::T_AND:
            andOp(r3, r1, r2);
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
    if (dynamic_cast<ast::var*>(node->child(0))) {
        // indirect
        lw(r1, 0, r1);
    }

    lw(r2, -node->child(1)->symbolTableEntry()->offset(), SP);
    if (dynamic_cast<ast::var*>(node->child(1))) {
        // indirect
        lw(r2, 0, r2);
    }

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

void StackCodeGeneratorVisitor::visit(ast::notFactor* node)
{
    Visitor::visit(node);

    text() << "% notFactor: " << node->symbolTableEntry()->name() << " := not " << node->child()->symbolTableEntry()->name() << endl;

    auto r1 = reg();
    auto r2 = reg();

    // load data
    lw(r1, -node->child()->symbolTableEntry()->offset(), SP);
    if (dynamic_cast<ast::var*>(node->child())) {
        // indirect
        lw(r1, 0, r1);
    }

    notOp(r2, r1);

    sw(-node->symbolTableEntry()->offset(), SP, r2);

    regPush(r2);
    regPush(r1);
}

void StackCodeGeneratorVisitor::visit(ast::assignStat* node)
{
    Visitor::visit(node);

    text() << "% assignStat: " << node->child(0)->symbolTableEntry()->name() << " := " << node->child(1)->symbolTableEntry()->name() << endl;

    auto r1 = reg();
    auto r2 = reg();

    // load the indirected offset from the lhs tempvar
    lw(r1, -node->child(0)->symbolTableEntry()->offset(), SP);

    // load the value to be copied
    lw(r2, -node->child(1)->symbolTableEntry()->offset(), SP);

    if (dynamic_cast<ast::var*>(node->child(1))) {
        // indirect, must fetch
        lw(r2, 0, r2);
    }

    // assign
    sw(0, r1, r2);

    regPush(r2);
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
    if (dynamic_cast<ast::var*>(node->child())) {
        // indirect
        lw(r1, 0, r1);
    }

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

void StackCodeGeneratorVisitor::visit(ast::getStat* node)
{
    Visitor::visit(node);

    auto table = node->closestSymbolTable();

    text() << "% getStat: " << node->child()->symbolTableEntry()->name() << endl;

    auto r1 = reg();

    // make the stack frame pointer point to the called function's stack frame
    addi(SP, SP, -table->size());

    // link buffer to stack
    addi(r1, ZR, "buf");
    sw(-8, SP, r1);

    // call getstr (result stored in buffer)
    jl(JL, "getstr");

    // convert int to string for output (result stored in r13/RV)
    jl(JL, "strint");

    // make the stack frame pointer point back to the current function's stack frame
    subi(SP, SP, -table->size());

    // store into indirected offset
    lw(r1, -node->child()->symbolTableEntry()->offset(), SP);
    sw(0, r1, RV);

    regPush(r1);
}

void StackCodeGeneratorVisitor::visit(ast::returnStat* node)
{
    Visitor::visit(node);

    text() << "% returnStat: " << node->child()->symbolTableEntry()->name() << endl;

    auto r1 = reg();

    lw(r1, -node->child()->symbolTableEntry()->offset(), SP);
    if (dynamic_cast<ast::var*>(node->child())) {
        // indirect
        lw(r1, 0, r1);
    }

    sw(-4, SP, r1);

    regPush(r1);
}

void StackCodeGeneratorVisitor::visit(ast::funcDef* node)
{
    auto function = node->symbolTableEntry();
    bool isFreeFunction = !function->parentTable()->parentEntry();
    std::string prefix;

    // for member functions, try to uniquely id them
    if (!isFreeFunction) {
        prefix += function->parentTable()->parentEntry()->name();
    }

    // create the tag to jump onto
    // and copy the jumping-back address value in the called function's stack frame
    text(prefix + function->name()) << "% funcDef: " << prefix << function->name() << endl;
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

    auto table = node->closestSymbolTable();
    auto idNode = dynamic_cast<ast::id*>(node->child(0));
    auto entry = node->symbolTableEntry();
    auto previous = node->previous();

    text() << "% dataMember: " << idNode->token()->value << endl;

    auto r1 = reg();
    auto r2 = reg();

    // in the var's tempvar, we store an address to the *top* of where the data is located for this dataMember,
    // which is its -offset plus its size
    lw(r1, -node->parent()->symbolTableEntry()->offset(), SP);
    addi(r1, r1, -entry->offset() + entry->size());

    // if the previous dataMember refers to a class, we need to check if we should adjust the offset for inheritance
    if (previous && dynamic_cast<semantic::VariableType*>(previous->symbolTableEntry()->type())->type == semantic::Type::CLASS) {
        auto classEntry = (*table)[dynamic_cast<semantic::VariableType*>(previous->symbolTableEntry()->type())->className];
        auto link = classEntry->link();

        // first, check if this dataMember's entry is in the class' own symbol table
        // in this case, we do not need to adjust any offsets as all the vars live at the top of the stack frame
        auto e = std::find_if(link->begin(), link->end(),
                              [&entry](const std::pair<semantic::SymbolTableEntry::key_type, semantic::SymbolTable::entry_type>& e) {
                                  return e.first == entry->name();
                              });

        if (e == link->end()) {
            int lastSize = link->size();

            // if it's not, we should iterate over all the supers and try to find where it is
            for (const auto& super : classEntry->supers()) {
                // every time we come across a super, we move the stack pointer offset down the previous table's size
                addi(r1, r1, -lastSize);
                lastSize = super->link()->size();

                // now we check if our entry is in this super's table
                auto e = std::find_if(super->link()->begin(), super->link()->end(),
                                      [&entry](const std::pair<semantic::SymbolTableEntry::key_type, semantic::SymbolTable::entry_type>& e) {
                                          return e.first == entry->name();
                                      });

                // if it is, we're done!
                if (e != super->link()->end()) {
                    break;
                }
            }
        }
    }

    // for indices, we calculate an offset amount to move down from the top of the data
    if (node->child(1)->child()) {
        int size = node->parent()->symbolTableEntry()->size();
        auto type = dynamic_cast<semantic::VariableType*>(entry->type());
        int i = 0;

        for (auto index = node->child(1)->child(); index != nullptr; index = index->next(), ++i) {
            text() << "% dataMember: index " << index->symbolTableEntry()->name() << endl;

            // for every index, we load the int index value from the indice's tempvar
            lw(r2, -index->symbolTableEntry()->offset(), SP);
            if (dynamic_cast<ast::var*>(index)) {
                // indirect
                lw(r2, 0, r2);
            }

            // we multiply the offset by the total column size for this index,
            // which is all dimensions after it multiplied
            for (int j = i + 1; j < type->indices.size(); ++j) {
                muli(r2, r2, type->indices[j]);
            }

            // finally, we mulitply the whole offset by the cell size
            muli(r2, r2, size);

            // and adjust the var offset accordingly by moving it down to the correct position,
            // where it now points at the top of the data cell
            sub(r1, r1, r2);
        }
    }

    sw(-node->parent()->symbolTableEntry()->offset(), SP, r1);

    regPush(r2);
    regPush(r1);
}

void StackCodeGeneratorVisitor::visit(ast::fCall* node)
{
    Visitor::visit(node);

    auto table = node->closestSymbolTable();
    auto function = node->symbolTableEntry();
    bool isFreeFunction = !function->parentTable()->parentEntry();
    std::string prefix;

    // for member functions, try to uniquely id them
    if (!isFreeFunction) {
        prefix += function->parentTable()->parentEntry()->name();
    }

    text() << "% fCall: " << prefix << function->name() << endl;

    auto r1 = reg();

    auto currentParam = function->parameters().begin();

    for (auto aParam = node->child(1)->child(); aParam != nullptr; aParam = aParam->next()) {
        text() << "%% fparam begin: " << (*currentParam)->name() << endl;

        lw(r1, -aParam->symbolTableEntry()->offset(), SP);
        if (dynamic_cast<ast::var*>(aParam)) {
            // indirect
            lw(r1, 0, r1);
        }

        sw(-table->size() - (*currentParam)->offset(), SP, r1);

        text() << "% fparam end: " << (*currentParam)->name() << endl;
        currentParam++;
    }

    lw(r1, -node->parent()->symbolTableEntry()->offset(), SP);

    // make the stack frame pointer point to the called function's stack frame
    addi(SP, SP, -table->size());

    // if this function has a this ptr, set it
    if (auto thisVar = (*function->link())["_this"]) {
        sw(-thisVar->offset(), SP, r1);
    }

    // jump to the called function's code
    // here the function's name is the label
    // TODO: unique names? should we use label() for function names? perhaps create in funcDef and store in entry
    jl(JL, prefix + function->name());

    // upon jumping back, set the stack frame pointer back to the current function's stack frame
    subi(SP, SP, -table->size());

    // copy the return value in memory space to store it on the current stack frame
    // to evaluate the expression in which it is
    lw(r1, -table->size() - 4, SP);
    sw(-node->child(1)->symbolTableEntry()->offset(), SP, r1);

    // hack: for indirection to work, we set the var's offset to the address of our hidden tempvar
    addi(r1, SP, -node->child(1)->symbolTableEntry()->offset());
    sw(-node->parent()->symbolTableEntry()->offset(), SP, r1);

    regPush(r1);
}

void StackCodeGeneratorVisitor::visit(ast::ifStat* node)
{
    text() << "% ifStat: begin" << endl;

    auto elseLabel = label("else");
    auto endifLabel = label("endif");

    // condition: call visitor on 1st child node (expression)
    node->child(0)->accept(this);

    // evaluate: if false, jump to else. if true, carry on
    auto r1 = reg();
    lw(r1, -node->child(0)->symbolTableEntry()->offset(), SP);
    bz(r1, elseLabel);
    regPush(r1);

    // then: call visitor on then statBlock
    text() << "% ifStat: then" << endl;
    node->child(1)->accept(this);
    j(endifLabel); // done, bail out to endif

    // else: call visitor on else statBlock, tag it
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
    text() << "% forStat: " << dynamic_cast<ast::id*>(node->child(1))->token()->value << " := " << node->child(2)->symbolTableEntry()->name() << endl;
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

void StackCodeGeneratorVisitor::muli(const std::string& dest, const std::string& source, const int& immediate)
{
    text() << "muli " << dest << ',' << source << ',' << immediate << endl;
}

void StackCodeGeneratorVisitor::div(const std::string& dest, const std::string& source, const std::string& offset)
{
    text() << "div " << dest << ',' << source << ',' << offset << endl;
}

void StackCodeGeneratorVisitor::andOp(const std::string& dest, const std::string& source, const std::string& offset)
{
    text() << "and " << dest << ',' << source << ',' << offset << endl;
}

void StackCodeGeneratorVisitor::orOp(const std::string& dest, const std::string& source, const std::string& offset)
{
    text() << "or " << dest << ',' << source << ',' << offset << endl;
}

void StackCodeGeneratorVisitor::notOp(const std::string& dest, const std::string& op)
{
    text() << "not " << dest << ',' << op << endl;
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
    return dataStream_ << std::left << std::setw(indentLength_) << (' ' + label);
}

std::ostream& StackCodeGeneratorVisitor::text()
{
    return textStream_ << indent_;
}

std::ostream& StackCodeGeneratorVisitor::text(const std::string& label)
{
    return textStream_ << std::left << std::setw(indentLength_) << (' ' + label);
}

std::string StackCodeGeneratorVisitor::label(const std::string& label)
{
    return label + std::to_string(labels_[label]++);
}

}}
