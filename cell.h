#pragma once

#include "common.h"
#include "formula.h"

#include <functional>
#include <unordered_set>
#include <optional>

class Sheet;

class Cell : public CellInterface {
public:
    Cell(Sheet& sheet);
    ~Cell() override;

    void Set(std::string text);
    void Clear();

    Value GetValue() const override;
    std::string GetText() const override;
    std::vector<Position> GetReferencedCells() const override;

    bool IsReferenced() const;

private:
    class Impl;
    class EmptyImpl;
    class TextImpl;
    class FormulaImpl;

    Sheet& sheet_;
    std::unique_ptr<Impl> impl_;

    // список €чеек, от которых зависит данна€
    std::unordered_set<Cell*> parents_;
    // список €чеек, которые завис€т от данной
    std::unordered_set<Cell*> children_;

    // ѕровер€ем циклические зависимости
    void HashCycles(const std::vector<Position>& parents, const Cell* root);

    // ‘ункции дл€ работы с кешем
    void DeleteParents();
    void SetParents();
    void ClearChildren();
    void ClearCache();
};