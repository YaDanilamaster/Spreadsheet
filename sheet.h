#pragma once

#include "cell.h"
#include "common.h"

#include <algorithm>
#include <functional>
#include <iostream>
#include <optional>
#include <vector>

class Sheet : public SheetInterface {
public:
    ~Sheet() = default;

    void SetCell(Position pos, std::string text) override;

    const CellInterface* GetCell(Position pos) const override;
    CellInterface* GetCell(Position pos) override;

    void ClearCell(Position pos) override;

    Size GetPrintableSize() const override;

    void PrintValues(std::ostream& output) const override;
    void PrintTexts(std::ostream& output) const override;

    // получаем ячейку даже если ее не было
    CellInterface* ObtainCell(Position pos);

private:
    std::vector<std::vector<std::unique_ptr<Cell>>> cells_;
    // чтобы не вычислять пустые ячейки при каждом изменении 
    // будем хранить по индексам строк и столбцов количество не пустых ячеек в них
    std::vector<int> rows_fit_;
    std::vector<int> cols_fit_;
    Size size_;

    void AddColumn(int count);
    void AddRow(int count);

    // урезает область по минимальным не пустым ячейкам
    void ShrinkToFit();

    template <typename Fn>
    void Print(std::ostream& output, Fn fn) const {
        for (auto& row : cells_) {
            bool need_print = false;
            for (auto& col : row) {
                if (need_print) output << '\t';
                if (col) {
                    fn(col.get());
                }
                need_print = true;
            }
            output << '\n';
        }
    }
};