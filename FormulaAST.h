#pragma once

#include "FormulaBaseListener.h"
#include "FormulaLexer.h"
#include "FormulaParser.h"
#include "common.h"
#include "formula.h"


#include <forward_list>
#include <functional>
#include <stdexcept>
#include <cassert>
#include <cmath>
#include <memory>
#include <optional>
#include <sstream>


namespace ASTImpl {
class Expr;
}

class ParsingError : public std::runtime_error {
    using std::runtime_error::runtime_error;
};

class FormulaAST {
public:
    explicit FormulaAST(std::unique_ptr<ASTImpl::Expr> root_expr,
                        std::forward_list<Position> cells);
    FormulaAST(FormulaAST&&) = default;
    FormulaAST& operator=(FormulaAST&&) = default;
    ~FormulaAST();

    double Execute(const SheetInterface& sheet) const;
    void PrintCells(std::ostream& out) const;
    void Print(std::ostream& out) const;
    void PrintFormula(std::ostream& out) const;

    std::forward_list<Position>& GetCells();
    const std::forward_list<Position>& GetCells() const;

private:
    std::unique_ptr<ASTImpl::Expr> root_expr_;

    // physically stores cells so that they can be
    // efficiently traversed without going through
    // the whole AST
    std::forward_list<Position> cells_;
};

FormulaAST ParseFormulaAST(std::istream& in);
FormulaAST ParseFormulaAST(const std::string& in_str);