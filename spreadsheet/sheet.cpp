#include "sheet.h"

#include "cell.h"
#include "common.h"

#include <algorithm>
#include <functional>
#include <iostream>
#include <optional>

using namespace std::literals;

Sheet::~Sheet() {;}

void Sheet::SetCell(Position pos, std::string text) {
    
    CheckValid(pos);
   
    bool is_formula = IsFormula(text);
    
    if (is_formula) {
      
        std::unique_ptr<FormulaInterface> formula = ParseFormula(text.substr(1));

        std::vector<Position> outgoing_edges = formula->GetReferencedCells();
        if (outgoing_edges.size() > 0) {
            if (!IsNoCiclesCreate(pos, outgoing_edges)) {
                throw CircularDependencyException("Cicle!!!");
            }
        }
     
    }
  
    MayBeChangeSize(pos);
    
    if (GetCell(pos) == nullptr) {
        AddNewCell(pos);
    }
    else{
        if (cells_[pos.row][pos.col]->HasValue()){
    
            std::set<Position> parents;
            GetAllParents(pos, parents);
            parents.insert(pos);
            Invalide_Values(parents);
        } 
       
        RemoveOldEdges(pos);
    }

    cells_[pos.row][pos.col]->Set(text, is_formula);
    if (is_formula){
        AddNewEdges(pos);
    }
    
}

const CellInterface* Sheet::GetCell(Position pos) const {
    
    CheckValid(pos);

    if (pos.row <= max_row_ && pos.col <= max_col_) {
        return cells_.at(pos.row).at(pos.col).get();
    }
    else {
        return nullptr;
    }
}
CellInterface* Sheet::GetCell(Position pos) {
    
    CheckValid(pos);
    
    if (pos.row <= max_row_ && pos.col <= max_col_) {
        return cells_[pos.row][pos.col].get();
    }
    else {
        return nullptr;
    }
}

void Sheet::ClearCell(Position pos) {
    
    CheckValid(pos);

    if (pos.row > max_row_ || pos.col > max_col_) {
         return;
    }


    if (GetCell(pos) == nullptr) {
        return;
    }
    if (cells_[pos.row][pos.col]->IsReferenced()) {
            std::set<Position> parents;
            GetAllParents(pos,parents);
            Invalide_Values(parents);
        }
    RemoveOldEdges(pos);
    cells_[pos.row][pos.col] = nullptr;
    
  
    --filled_rows_[pos.row];
    if (filled_rows_[pos.row] == 0) {
        filled_rows_.erase(pos.row);
        if (filled_rows_.size() == 0) {
            max_row_ = -1;
        }   
        else {
            if (max_row_ == pos.row) {
         
                max_row_ = filled_rows_.rbegin()->first;

            }
        }
    }
    
    
    --filled_cols_[pos.col];
    if (filled_cols_[pos.col] == 0) {
        filled_cols_.erase(pos.col);
        if (filled_cols_.size() == 0) {
            max_col_ = -1;
        }
        else{
            if (max_col_ == pos.col) {
                max_col_ = filled_cols_.rbegin()->first;
            }
        }
    }
    
}

Size Sheet::GetPrintableSize() const {
    
    Size res = {max_row_ + 1 , max_col_ + 1};
    return res;
    
}





void Sheet::PrintValues(std::ostream& output) const {
   
    for (int i = 0; i <= max_row_; ++i){
        for (int j = 0; j <=max_col_; ++j) {
            if  (cells_[i][j] != nullptr){
                auto value = cells_[i][j]->GetValue();
                std::visit([&output](const auto& val) {
                using T = std::decay_t<decltype(val)>; 
                if constexpr (std::is_same_v<T, std::string>) {
                    output <<  val;
                } 
                else {
                    if constexpr (std::is_same_v<T, double>) {
                        output << val;
                    } else 
                        if constexpr (std::is_same_v<T, FormulaError>) {
                            output<< val;
                        }
                }
                }, value);
            }
            else{
                ;
            }
            if (j<max_col_) {
                    output<<'\t';
                }
        }
        output<<"\n";
    }

}
void Sheet::PrintTexts(std::ostream& output) const {
  
    for (int i = 0; i <= max_row_; ++i){
        for (int j = 0; j <=max_col_; ++j) {
            if  (cells_[i][j] != nullptr){
                output<<cells_[i][j] -> GetText();
                
            }
            else{
                ;
            }
        if (j<max_col_){
                    output<<'\t';
             }
        }
        output<<'\n';
    }
    
}

std::unique_ptr<SheetInterface> CreateSheet() {
    return std::make_unique<Sheet>();
}

void Sheet::AddEmptyColumns(int n) {
    for (auto& row : cells_) {
        for (int i = 0; i < n; ++i) {
            row.emplace_back(nullptr); 
        }
    }
}
void Sheet::AddEmptyRows(int n) {
    
    for (int i = 0; i < n; ++i) {
    std::vector<std::unique_ptr<Cell>> empty_row;
    
    for (size_t i = 0; i < cells_[0].size(); ++i) {
        empty_row.emplace_back(nullptr); 
    }
        cells_.push_back(move(empty_row)); 
    }
}
void Sheet::AddEmptyRows(int row, int col) {
    
    for (int i = 0; i < row; ++i) {
    std::vector<std::unique_ptr<Cell>> empty_row;
    
    for (int i = 0; i < col; ++i) {
        empty_row.emplace_back(nullptr); 
    }
        cells_.push_back(move(empty_row)); 
    }
}
//////////NEW//////////////////

bool Sheet::IsFormula(const std::string& text) {
    if (text.size() == 0) {
        return false;
    }
     if (text[0] == '=' && text.size() > 1) {
        return true;
     }
     else {
        return false;
     }
}

 bool Sheet::IsNoCiclesCreate(Position pos, const std::vector<Position>& outgoing_edges) const{
    std::set<Position> parents;
   
    if (GetCell(pos) != nullptr) {
    
        GetAllParents(pos, parents);
    }
  
    parents.insert(pos);
    std::set<Position> validated_vertices;
  
    for( const Position edge : outgoing_edges)
    {
        if (parents.count(edge) == 1) {
            return false;
        }
        else {
          
            if (HasNoLinkOnParents(edge, validated_vertices, parents)) {
              
                validated_vertices.insert(edge);
            }
            else {
                
                return false;
            }
        }
    }
   
    return true;

}

bool Sheet::HasNoLinkOnParents(Position pos, std::set<Position>& validated_vertices, const std::set<Position>& parents) const {
     
    if (GetCell(pos)!=nullptr) {
        for ( const Position edge : cells_[pos.row][pos.col]->GetReferencedCells()){
           
            if (parents.count(edge) == 1) {
                return false;
            }
            else {
                if (HasNoLinkOnParents(edge,validated_vertices, parents)) {
                    validated_vertices.insert(edge);
                }
                else {
                    return false;
                }
            }
        }
    }
    return true;
}

void Sheet::GetAllParents(Position pos, std::set<Position>& parents) const {
    
    for (const Position edge : cells_[pos.row][pos.col]->GetIngoingEdges()) {
        if (cells_[edge.row][edge.col]->HasValue() && parents.count(edge) == 0){
            parents.insert(edge);
            GetAllParents(edge, parents);
        }
    }
    
}

void Sheet::Invalide_Values(const std::set<Position>& parents){
    for (const Position pos : parents) {
        cells_[pos.row][pos.col]->InvalideValue();
    }
}

void Sheet::RemoveOldEdges(Position pos) {
    for (Position edge : GetCell(pos) -> GetReferencedCells()){
        
        if (!edge.IsValid()){
            continue;
        }

        if (GetCell(edge)) {
            cells_[edge.row][edge.col] -> DeleteParent(pos);
        }
    }
}

void Sheet::AddNewEdges(Position pos) {
    for (Position edge : GetCell(pos) -> GetReferencedCells()){
        if (!edge.IsValid()){
            continue;
        }

        if (GetCell(edge) == nullptr) {
            MayBeChangeSize(edge);
            AddNewCell(edge);    
        }
        cells_[edge.row][edge.col] -> AddParent(pos);
    }
}
void Sheet::MayBeChangeSize(Position pos){
    if (cells_.size() == 0) {
        max_row_ = pos.row;
        max_col_ = pos.col;
        AddEmptyRows(pos.row + 1, pos.col +1);
    }
    else{
        if (pos.row > max_row_) {
            max_row_ = pos.row;
            int delta = pos.row - cells_.size() + 1;
            if (delta > 0) {
                AddEmptyRows(delta);
            }
        }
        if (pos.col > max_col_) {
        
            max_col_ = pos.col;
        
        
            int delta = pos.col - cells_[0].size() + 1;
            if (delta>0) {
                AddEmptyColumns(delta);
            }
        }
    }
}

void Sheet::AddNewCell(Position pos) {
    cells_[pos.row][pos.col] = std::make_unique<Cell>(*this);
     
    auto it1 = filled_rows_.find(pos.row);
    if (it1 == filled_rows_.end()) {
        filled_rows_.insert(std::make_pair(pos.row, 1));
    }
    else {
        ++filled_rows_[pos.row];
    }
  
    auto it2 = filled_cols_.find(pos.col);
    if (it2 == filled_cols_.end()) {
        filled_cols_.insert(std::make_pair(pos.col, 1));
    }
    else {
        ++filled_cols_[pos.col];
    }
}

void CheckValid(Position pos) {
    if (!pos.IsValid()) {
         throw InvalidPositionException("Position is WRONG!!!");
    }
}