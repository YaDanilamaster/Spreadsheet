#include "cell.h"
#include "sheet.h"


using namespace std::literals;


class Cell::Impl {
public:
	virtual CellInterface::Value GetValue(const SheetInterface& sheet)
	{
		return ""s;
	}

	virtual std::string GetText()
	{
		return ""s;
	}

	virtual std::vector<Position> GetReferencedCells() const
	{
		return {};
	}

	virtual void ClearCahce()
	{
	}

	virtual ~Impl() = default;
protected:
	Impl() = default;
};

class Cell::EmptyImpl : public Impl {
public:
	EmptyImpl() = default;
};


class Cell::TextImpl : public Impl {
public:
	TextImpl(const std::string& text) : text_(text)
	{
	}

	CellInterface::Value GetValue(const SheetInterface& sheet) override {
		if (text_[0] == ESCAPE_SIGN) {
			return text_.substr(1);
		}
		return text_;
	}

	std::string GetText() override {
		return text_;
	}
private:
	std::string text_;
};


class Cell::FormulaImpl : public Impl {
public:
	FormulaImpl(const std::string& text)
		: formula_(ParseFormula(text))
	{
	}

	CellInterface::Value GetValue(const SheetInterface& sheet) override {
		if (!cache_) {
			cache_ = formula_->Evaluate(sheet);
		}

		return std::visit([](auto val) {
			return CellInterface::Value(val);
			}, cache_.value());
	}

	std::string GetText() override {
		return FORMULA_SIGN + formula_->GetExpression();
	}

	std::vector<Position> GetReferencedCells() const override {
		return formula_->GetReferencedCells();
	}

	void ClearCahce() override {
		cache_.reset();
	}

private:
	std::unique_ptr<FormulaInterface> formula_;
	std::optional<FormulaInterface::Value> cache_;
};



// Реализуйте следующие методы
Cell::Cell(Sheet& sheet)
	: sheet_(sheet)
	, impl_(std::make_unique<EmptyImpl>())
{}

Cell::~Cell() = default;

// Задаёт содержимое ячейки. Если текст начинается со знака "=", то он
// интерпретируется как формула. Уточнения по записи формулы:
// * Если текст содержит только символ "=" и больше ничего, то он не считается
// формулой
// * Если текст начинается с символа "'" (апостроф), то при выводе значения
// ячейки методом GetValue() он опускается. Можно использовать, если нужно
// начать текст со знака "=", но чтобы он не интерпретировался как формула.
void Cell::Set(std::string text) {
	if (impl_ != nullptr && text == impl_->GetText()) {
		return;
	}

	//Clear();

	if (text.empty()) {
		impl_ = std::make_unique<EmptyImpl>();
	}
	else if (text[0] == FORMULA_SIGN) {
		auto temp_impl = std::make_unique<FormulaImpl>(text.substr(1));
		HashCycles(temp_impl->GetReferencedCells(), this);
		impl_ = std::move(temp_impl);
	}
	else {
		impl_ = std::make_unique<TextImpl>(std::move(text));
	}

	DeleteParents();
	SetParents();
	ClearChildren();
}

void Cell::Clear() {
	impl_ = std::make_unique<EmptyImpl>();
	DeleteParents();
	ClearChildren();
}

Cell::Value Cell::GetValue() const {
	return impl_->GetValue(sheet_);
}
std::string Cell::GetText() const {
	return impl_->GetText();
}

std::vector<Position> Cell::GetReferencedCells() const
{
	return impl_->GetReferencedCells();
}

bool Cell::IsReferenced() const
{
	return !children_.empty();
}

void Cell::HashCycles(const std::vector<Position>& parents, const Cell* root)
{
	for (const Position& check_position : parents) {

		if (Cell* parent = static_cast<Cell*>(sheet_.GetCell(check_position))) {
			if (parent == root) {
				throw CircularDependencyException("Cycle found!"s);
			}
			parent->HashCycles(parent->GetReferencedCells(), root);
		}
	}
}

void Cell::DeleteParents()
{
	for (Cell* parent : parents_) {
		parent->children_.erase(this);
	}
	parents_.clear();
}

void Cell::SetParents()
{
	for (const Position& parent_pos : GetReferencedCells()) {
		Cell* parent = static_cast<Cell*>(sheet_.ObtainCell(parent_pos));
		parent->children_.insert(this);
		parents_.insert(parent);
	}
}

void Cell::ClearChildren()
{
	ClearCache();
	for (Cell* child : children_) {
		// вызываем рекурсивно для каждого ребенка
		child->ClearChildren();
	}
}

void Cell::ClearCache()
{
	impl_->ClearCahce();
}




