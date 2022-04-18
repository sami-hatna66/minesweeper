//
//  Cell.cpp
//  minesweeper
//
//  Created by Sami Hatna on 07/04/2022.
//

#include "Cell.hpp"

// Initialize cell with default settings
Cell::Cell() {
    this->state = CellState::unopened;
    this->hasMine = false;
    this->adjacentNum = 0;
    this->row = 0;
    this->col = 0;
}

// Collection of getters and setters for private attributes

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

int Cell::getRow() {
    return this->row;
}

void Cell::setRow(int row) {
    this->row = row;
}

int Cell::getCol() {
    return this->col;
}

void Cell::setCol(int col) {
    this->col = col;
}
