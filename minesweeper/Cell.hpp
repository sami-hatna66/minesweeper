//
//  Cell.hpp
//  minesweeper
//
//  Created by Sami Hatna on 07/04/2022.
//

#ifndef Cell_hpp
#define Cell_hpp

#include <stdio.h>

enum class CellState {opened, unopened, flagged};

class Cell {
private:
    CellState state;
    bool hasMine;
    int adjacentNum;
public:
    Cell();
    CellState getState();
    void setState(CellState newState);
    bool getHasMine();
    void setHasMine(bool newVal);
    int getAdjacentNum();
    void setAdjacentNum(int newVal);
};

#endif /* Cell_hpp */
