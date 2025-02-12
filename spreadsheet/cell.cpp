#include "cell.h"

#include <cassert>
#include <iostream>
#include <string>
#include <optional>

// Реализуйте следующие методы

void Cell::Set(std::string text, bool is_formula) {
    
   
    outgoing_edges_.clear();
    

    if (is_formula) {
        is_it_formula_ = true;
        formula_ = ParseFormula(text.substr(1));
        text_ = FORMULA_SIGN + formula_->GetExpression();
        outgoing_edges_ = formula_ -> GetReferencedCells();

    }
    else{
        is_it_formula_ = false;
        text_ = text;
        formula_ = nullptr;
    }
 
}

void Cell::Clear() {
    is_it_formula_ = false;
    text_.clear();
    formula_ = nullptr;
    outgoing_edges_.clear();
    value_.reset();
}

CellInterface::Value ConvertValue(const FormulaInterface::Value& formulaValue) {
    
    return std::visit([](auto&& arg) -> CellInterface::Value {
        using T = std::decay_t<decltype(arg)>;
        if constexpr (std::is_same_v<T, double>) {
            
            return arg;  
        } else if constexpr (std::is_same_v<T, FormulaError>) {
           
            return arg;  
        }
    }, formulaValue);
}

bool Cell::HasValue() const{
    return !(value_ == std::nullopt);  
}

CellInterface::Value Cell::GetValue() const  {

    if (HasValue()){
        return *value_;
    }
    
    CellInterface::Value res;

    if (is_it_formula_){
        res = ConvertValue(formula_->Evaluate(sheet_interface_));
        value_ = std::make_optional(res);
    }
    else{
      
        if (IsNonEmptyAndNotOnlySpaces(text_)){
            if (text_[0] == '\'' ) {
                res = text_.substr(1);
                value_ = std::make_optional(res);
            }
            else{
                
                if (CanConvertToDouble(text_)){
                    res = strtod(text_.c_str(), nullptr);
                    value_ = std::make_optional(res);
                }
                else{
               
                    res = text_;
                    value_ = std::make_optional(res);
                }
            }
        }
        else{
            value_ = 0.0;
            res = 0.0;
        }
    }
    return res;
}

std::string Cell::GetText() const {
    return text_;
}



bool Cell::IsReferenced() const {
    if (ingoing_edges_.empty()) {
        return false;
    }
    else{
        return true;
    }   
}



void Cell::InvalideValue() {
    value_.reset();
}

void Cell::DeleteParent(Position pos) {
    ingoing_edges_.erase(pos);
}

void Cell::AddParent(Position pos) {
    ingoing_edges_.insert(pos);
}

std::set<Position> Cell::GetIngoingEdges() {
    return ingoing_edges_;
}

bool IsNonEmptyAndNotOnlySpaces(const std::string& str) {
   
    if (str.empty()) {
        return false;
    }

    return std::any_of(str.begin(), str.end(), [](unsigned char c) { return !std::isspace(c); });
}

bool CanConvertToDouble(const std::string& str) {
    
    size_t start = str.find_first_not_of(" \t");
    size_t end = str.find_last_not_of(" \t");

    std::string trimmed = str.substr(start, end - start + 1);

    char* endPtr;
    errno = 0; 
    std::strtod(trimmed.c_str(), &endPtr);
    
    return errno == 0 && endPtr == trimmed.c_str() + trimmed.length();
}

std::vector<Position> Cell::GetReferencedCells() const{
    return outgoing_edges_;
}
