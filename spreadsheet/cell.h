#pragma once

#include "common.h"
#include "formula.h"
#include "sheet.h"

#include <algorithm>
#include <functional>
#include <unordered_set>
#include <set>
#include <optional>

class Sheet;

class Cell : public CellInterface {
public:
    
    std::vector<Position> GetReferencedCells() const override;
    std::set<Position> GetIngoingEdges();
    
  
    Cell(Sheet& sheet):is_it_formula_(false),text_(""),formula_(nullptr),sheet_(sheet),sheet_interface_(static_cast< SheetInterface&>(sheet)) {
    
}
    ~Cell() override = default;

    void Set(std::string text, bool is_formula);
    void Clear();

    CellInterface::Value GetValue() const override;
    std::string GetText() const override;
    
    bool IsReferenced() const;
    
    
    
    bool HasValue() const;
    void InvalideValue();

    void DeleteParent(Position pos);
    void AddParent(Position pos);

private:

    bool is_it_formula_ = false;
    std::string text_;
    std::unique_ptr<FormulaInterface> formula_;
    
    Sheet& sheet_;
    SheetInterface& sheet_interface_;

    std::vector<Position> outgoing_edges_;
    std::set<Position> ingoing_edges_;
    
    mutable std::optional<CellInterface::Value> value_;

    

//можете воспользоваться нашей подсказкой, но это необязательно.
/*    class Impl;
    class EmptyImpl;
    class TextImpl;
    class FormulaImpl;
    std::unique_ptr<Impl> impl_;
*/
};

bool IsNonEmptyAndNotOnlySpaces(const std::string& str);
bool CanConvertToDouble(const std::string& str);
