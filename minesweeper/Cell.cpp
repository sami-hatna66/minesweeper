//
//  Cell.cpp
//  minesweeper
//
//  Created by Sami Hatna on 07/04/2022.
//

#include "Cell.hpp"

Cell::Cell() {
    this->state = CellState::unopened;
    this->hasMine = false;
    this->adjacentNum = 0;
}

CellState Cell::getState() {
    return this->state;
}

void Cell::setState(CellState newState) {
    this->state = newState;
}

bool Cell::getHasMine() {
    return this->hasMine;
}

void Cell::setHasMine(bool newVal) {
    this->hasMine = newVal;
}

int Cell::getAdjacentNum() {
    return this->adjacentNum;
}

void Cell::setAdjacentNum(int newVal) {
    this->adjacentNum = newVal;
}
