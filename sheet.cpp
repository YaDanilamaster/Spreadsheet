#include "sheet.h"


using namespace std::literals;

void Sheet::SetCell(Position pos, std::string text) 
{
    if (!pos.IsValid()) {
        throw InvalidPositionException("");
    }

    if (pos.row >= size_.rows) {
        AddRow(pos.row - size_.rows + 1);
    }
    if (pos.col >= size_.cols) {
        AddColumn(pos.col - size_.cols + 1);
    }

    auto& curr_cell = cells_[pos.row][pos.col];
    // увеличиваем индексацию строки и столбца на одну запись
    if (!curr_cell) {
        ++rows_fit_[pos.row];
        ++cols_fit_[pos.col];
        curr_cell = std::make_unique<Cell>(*this);
    }
        
    curr_cell->Set(std::move(text));
}

const CellInterface* Sheet::GetCell(Position pos) const 
{
    if (!pos.IsValid()) {
        throw InvalidPositionException("");
    }

    if (pos.row >= size_.rows || pos.col >= size_.cols) {
        return nullptr;
    }

    return cells_[pos.row][pos.col].get();
}

CellInterface* Sheet::GetCell(Position pos) 
{
    if (!pos.IsValid()) {
        throw InvalidPositionException("");
    }

    if (pos.row >= size_.rows || pos.col >= size_.cols) {
        return nullptr;
    }

    return cells_[pos.row][pos.col].get();
}

void Sheet::ClearCell(Position pos) 
{
    if (!pos.IsValid()) {
        throw InvalidPositionException("");
    }

    Cell* curr_cell = static_cast<Cell*>(GetCell(pos));
    // уменьшаем индексацию строки и столбца на одну запись
    if (curr_cell != nullptr) {
        --rows_fit_[pos.row];
        --cols_fit_[pos.col];
        curr_cell->Clear();
    }
    // надо урезать область по минимальным данным
    ShrinkToFit();
}

Size Sheet::GetPrintableSize() const 
{
    return { size_.rows, size_.cols };
}

void Sheet::PrintValues(std::ostream& output) const 
{
    Print(output, [&output](const Cell* cell) {
        std::visit(
            [&output](const auto& x) {
                output << x;
            },
            cell->GetValue());
        });
}

void Sheet::PrintTexts(std::ostream& output) const 
{
    Print(output, [&output](const Cell* cell) {
            output << cell->GetText();
        });
}

CellInterface* Sheet::ObtainCell(Position pos)
{
    CellInterface* cell = GetCell(pos);
    if (cell == nullptr) {
        SetCell(pos, ""s);
        cell = GetCell(pos);
    }
    return static_cast<Cell*>(cell);
}

void Sheet::AddColumn(int count)
{
    int new_size = size_.cols + count;
    for (auto& row : cells_) {
        row.resize(new_size);
    }
    size_.cols = new_size;
    cols_fit_.resize(new_size);
}

void Sheet::AddRow(int count)
{
    int new_size = size_.rows + count;
    cells_.resize(new_size);

    for (int i = size_.rows; i < new_size; ++i) {
        cells_[i].resize(size_.cols);
    }

    size_.rows = new_size;
    rows_fit_.resize(new_size);
}

void Sheet::ShrinkToFit()
{
    size_t row_count = size_.rows;
    if (row_count > 0) {
        while (row_count > 0 && rows_fit_[row_count - 1] == 0) {
            --row_count;
        }
        cells_.resize(row_count);
        rows_fit_.resize(row_count);
        size_.rows = row_count;
    }

    size_t col_count = size_.cols;
    if (col_count > 0) {
        while (col_count > 0 && cols_fit_[col_count - 1] == 0) {
            --col_count;
        }

        for (auto& row : cells_) {
            row.resize(col_count);
        }
        cols_fit_.resize(col_count);
        size_.cols = col_count;
    }
}

std::unique_ptr<SheetInterface> CreateSheet() {
    return std::make_unique<Sheet>();
}