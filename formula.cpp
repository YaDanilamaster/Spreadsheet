#include "common.h"
#include "formula.h"
#include "FormulaAST.h"

using namespace std::literals;

FormulaError::FormulaError(Category category)
	: category_(category) 
{
}

FormulaError::Category FormulaError::GetCategory() const {
	return category_;
}

bool FormulaError::operator==(FormulaError rhs) const {
	return category_ == rhs.category_;
}

std::string_view FormulaError::ToString() const {
	switch (category_) {
	case Category::Ref:
		return "#REF!"sv;
	case Category::Value:
		return "#VALUE!"sv;
	case Category::Div0:
		return "#DIV/0!"sv;
	default:
		assert(false);
		return ""sv;
	}
}

std::ostream& operator<<(std::ostream& output, FormulaError fe) {
	return output << fe.ToString();
}

namespace {
	class Formula : public FormulaInterface {
	public:
		explicit Formula(std::string expression)
			: ast_([&expression]() {
			try {
				return ParseFormulaAST(expression);
			}
			catch (std::exception& e) {
				throw FormulaException(e.what());
			}
				}())
		{
			Position prev = Position::NONE;
			for (auto& cell : ast_.GetCells()) {
				if (cell.IsValid() && !(cell == prev)) {
					referenced_cells_.push_back(std::move(cell));
					prev = cell;
				}
			}
		}

		Value Evaluate(const SheetInterface& sheet) const override
		{
			try {
				return ast_.Execute(sheet);
			}
			catch (FormulaError& e) {
				return e;
			}
		}

		std::string GetExpression() const override
		{
			std::stringstream oss;
			ast_.PrintFormula(oss);
			return oss.str();
		}

		std::vector<Position> GetReferencedCells() const override {
			return referenced_cells_;
		}

	private:
		FormulaAST ast_;
		std::vector<Position> referenced_cells_;
	};
}  // namespace

std::unique_ptr<FormulaInterface> ParseFormula(std::string expression) {
	return std::make_unique<Formula>(std::move(expression));
}