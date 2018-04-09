#pragma once

#include "moonshine/Visitor.h"
#include "moonshine/syntax/Node.h"
#include "moonshine/semantic/Type.h"

#include <sstream>
#include <string>
#include <stack>

namespace moonshine { namespace code {

class StackCodeGeneratorVisitor : public Visitor
{
public:
    inline VisitorOrder order() override
    {
        return VisitorOrder::NONE;
    };

    StackCodeGeneratorVisitor(std::ostream& dataStream, std::ostream& textStream);

    void visit(ast::prog* node) override;
    void visit(ast::statBlock* node) override;
    void visit(ast::num* node) override;
    void visit(ast::var* node) override;
    void visit(ast::addOp* node) override;
    void visit(ast::multOp* node) override;
    void visit(ast::assignStat* node) override;
    void visit(ast::putStat* node) override;
    void visit(ast::returnStat* node) override;
    void visit(ast::funcDef* node) override;
    void visit(ast::dataMember* node) override;
    void visit(ast::fCall* node) override;
private:
    const std::string ZR = "r0";
    const std::string RV = "r13";
    const std::string SP = "r14";
    const std::string JL = "r15";

    const std::string indent_ = "          ";

    std::ostream& dataStream_;
    std::ostream& textStream_;

    std::stack<std::string> registers_;

    std::string reg();
    void regPush(const std::string& reg);

    std::ostream& data();
    std::ostream& data(const std::string& label);
    std::ostream& text();
    std::ostream& text(const std::string& label);

    // instructions
    void add(const std::string& dest, const std::string& source, const std::string& offset);
    void addi(const std::string& dest, const std::string& source, const std::string& immediate);
    void addi(const std::string& dest, const std::string& source, const int& immediate);
    void mul(const std::string& dest, const std::string& source, const std::string& offset);
    void subi(const std::string& dest, const std::string& source, const std::string& immediate);
    void subi(const std::string& dest, const std::string& source, const int& immediate);
    void lw(const std::string& dest, const int& offset, const std::string& source);
    void sw(const int& offset, const std::string& dest, const std::string& source);
    void jl(const std::string& store, const std::string& jmp);
    void jr(const std::string& jmp);
};

}}
