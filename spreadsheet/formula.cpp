#include "formula.h"

#include "FormulaAST.h"

#include <algorithm>
#include <cassert>
#include <cctype>
#include <sstream>

using namespace std::literals;

std::ostream& operator<<(std::ostream& output, FormulaError fe) {
    
    switch (fe.GetCategory()) {
    case FormulaError::Category::Ref:
        return output << "#REF!";
    case FormulaError::Category::Value:
        return output << "#VALUE!";
    case FormulaError::Category::Arithmetic:
        return output << "#ARITHM!";
    default:
        // Обработка случая, если категория не попадает ни под одну из перечисленных
        return output; // или вернуть некое дефолтное значение
}
}

namespace {
class Formula : public FormulaInterface {
public:
// Реализуйте следующие методы:
    explicit Formula(std::string expression) try
        : ast_(ParseFormulaAST(expression)) {
    }
    catch (const std::exception& exc) {
        std::cerr << "Exception: " << exc.what() << std::endl;
        throw FormulaException("Error parsing the formula: " + std::string(exc.what()));
    }
   
    Value Evaluate(const SheetInterface& sheet) const override{
        
        return ast_.Execute(sheet);
        
    }  
     
    std::string GetExpression() const override{
       std::ostringstream out; 
       ast_.PrintFormula(out); 
       return out.str(); 
    } 
    std::vector<Position> GetReferencedCells() const override{
        return ast_.GetReferencedCells();
    }


private:
    FormulaAST ast_;
};
}  // namespace

std::unique_ptr<FormulaInterface> ParseFormula(std::string expression) {
    return std::make_unique<Formula>(std::move(expression));
}