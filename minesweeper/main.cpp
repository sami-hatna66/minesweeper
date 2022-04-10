//
//  main.cpp
//  minesweeper
//
//  Created by Sami Hatna on 07/04/2022.
//

#include <iostream>
#include <SDL2/SDL.h>
#include <SDL_ttf.h>
#include <random> 
#include <array>
#include <map>
#include <vector>
#include "Cell.hpp"
using namespace std;

void init();
void initBoard();
void draw();
void close();
void renderText(char *inpText, int x, int y, SDL_Color color);
void leftClick(int x, int y);
void rightClick(int x, int y);
void resetBtnAction();

SDL_Window* win = NULL;
SDL_Surface* surface = NULL;
SDL_Renderer* render = NULL;

int boardHeight = 16;
int boardWidth = 30;

vector<vector<Cell>> gameBoard (boardWidth, vector<Cell>(boardHeight));

int highlightCoords[] = {-1, -1};

map<int, SDL_Color> colorMap = {
    {1, { static_cast<Uint8>(0), static_cast<Uint8>(0), static_cast<Uint8>(255) }},
    {2, { static_cast<Uint8>(2), static_cast<Uint8>(123), static_cast<Uint8>(0) }},
    {3, { static_cast<Uint8>(255), static_cast<Uint8>(0), static_cast<Uint8>(0) }},
    {4, { static_cast<Uint8>(0), static_cast<Uint8>(0), static_cast<Uint8>(123) }},
    {5, { static_cast<Uint8>(139), static_cast<Uint8>(69), static_cast<Uint8>(19) }},
    {6, { static_cast<Uint8>(102), static_cast<Uint8>(205), static_cast<Uint8>(170) }},
    {7, { static_cast<Uint8>(0), static_cast<Uint8>(0), static_cast<Uint8>(0) }},
    {8, { static_cast<Uint8>(105), static_cast<Uint8>(105), static_cast<Uint8>(105) }}
};

enum class Difficulties {beginner, intermediate, expert};
// width, height, mines
map <Difficulties, vector<int>> settingsMap = {
    {Difficulties::beginner, {9, 9, 10}},
    {Difficulties::intermediate, {16, 16, 40}},
    {Difficulties::expert, {30, 16, 99}}
};
Difficulties currentDifficulty = Difficulties::intermediate;

enum class GameStatus {alive, dead, complete};
GameStatus status = GameStatus::alive;

int numFlags = 40;
int flagCount = numFlags;

void init() {
    SDL_Init(SDL_INIT_VIDEO);
    TTF_Init();
    win = SDL_CreateWindow("Minesweeper", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 340, 390, SDL_WINDOW_SHOWN);
    initBoard();
}

void initBoard() {
    boardWidth = settingsMap[currentDifficulty][0];
    boardHeight = settingsMap[currentDifficulty][1];
    
    SDL_SetWindowSize(win, 20 + (20 * boardWidth), 70 + (20 * boardHeight));
    
    numFlags = settingsMap[currentDifficulty][2];
    flagCount = numFlags;
    status = GameStatus::alive;
    highlightCoords[0] = -1; highlightCoords[1] = -1;
    
    for (int i = 0; i < boardWidth; i++) {
        for (int j = 0; j < boardHeight; j++) {
            gameBoard[i][j].setState(CellState::unopened);
            gameBoard[i][j].setHasMine(false);
            gameBoard[i][j].setAdjacentNum(0);
        }
    }
    
    for (int i = 0; i < numFlags; i++) {
        random_device rd;
        mt19937 gen(rd());
        uniform_int_distribution<> distr1(0, boardWidth - 1);
        int randVal1 = distr1(gen);
        uniform_int_distribution<> distr2(0, boardHeight - 1);
        int randVal2 = distr2(gen);
        if (!gameBoard[randVal1][randVal2].getHasMine()) {
            gameBoard[randVal1][randVal2].setHasMine(true);
        }
        else {
            i -= 1;
        }
    }
    
    for (int i = 0; i < boardWidth; i++) {
        for (int j = 0; j < boardHeight; j++) {
            int count = 0;
            if (!gameBoard[i][j].getHasMine()) {
                if (j + 1 < boardHeight && gameBoard[i][j+1].getHasMine()) { count++; }
                if (j - 1 >= 0 && gameBoard[i][j-1].getHasMine()) { count++; }
                if (i + 1 < boardWidth && gameBoard[i+1][j].getHasMine()) { count++; }
                if (i - 1 >= 0 && gameBoard[i-1][j].getHasMine()) { count++; }
                if (i - 1 >= 0 && j + 1 < boardHeight && gameBoard[i-1][j+1].getHasMine()) { count++; }
                if (i - 1 >= 0 && j - 1 >= 0 && gameBoard[i-1][j-1].getHasMine()) { count++; }
                if (i + 1 < boardWidth && j + 1 < boardHeight && gameBoard[i+1][j+1].getHasMine()) { count++; }
                if (i + 1 < boardWidth && j - 1 >= 0 && gameBoard[i+1][j-1].getHasMine()) { count++; }
                gameBoard[i][j].setAdjacentNum(count);
            }
        }
    }
}

void leftClick(int x, int y) {
    if (x >= 0 && x < boardWidth && y >= 0 && y < boardHeight
        && gameBoard[x][y].getState() != CellState::flagged) {
        if (status == GameStatus::alive) {
            gameBoard[x][y].setState(CellState::opened);
            
            if (gameBoard[x][y].getHasMine()) {
                highlightCoords[0] = x; highlightCoords[1] = y;
                status = GameStatus::dead;
                for (int i = 0; i < boardWidth; i++) {
                    for (int j = 0; j < boardHeight; j++) {
                        if (gameBoard[i][j].getHasMine()) {
                            gameBoard[i][j].setState(CellState::opened);
                        }
                    }
                }
            }
            else if (gameBoard[x][y].getAdjacentNum() == 0) {
                if (x + 1 < boardWidth
                    && gameBoard[x + 1][y].getState() == CellState::unopened
                    && !gameBoard[x + 1][y].getHasMine()) {
                    if (gameBoard[x + 1][y].getAdjacentNum() == 0) {
                        leftClick(x + 1, y);
                    }
                    else {
                        gameBoard[x + 1][y].setState(CellState::opened);
                    }
                }
                if (x - 1 >= 0
                    && gameBoard[x - 1][y].getState() == CellState::unopened
                    && !gameBoard[x - 1][y].getHasMine()) {
                    if (gameBoard[x - 1][y].getAdjacentNum() == 0) {
                        leftClick(x - 1, y);
                    }
                    else {
                        gameBoard[x - 1][y].setState(CellState::opened);
                    }
                }
                if (y + 1 < boardHeight
                    && gameBoard[x][y + 1].getState() == CellState::unopened
                    && !gameBoard[x][y + 1].getHasMine()) {
                    if (gameBoard[x][y + 1].getAdjacentNum() == 0) {
                        leftClick(x, y + 1);
                    }
                    else {
                        gameBoard[x][y + 1].setState(CellState::opened);
                    }
                }
                if (y - 1 >= 0
                    && gameBoard[x][y - 1].getState() == CellState::unopened
                    && !gameBoard[x][y - 1].getHasMine()) {
                    if (gameBoard[x][y - 1].getAdjacentNum() == 0) {
                        leftClick(x, y - 1);
                    }
                    else {
                        gameBoard[x][y - 1].setState(CellState::opened);
                    }
                }
                if (x - 1 >= 0 && y + 1 < boardHeight
                    && gameBoard[x - 1][y + 1].getState() == CellState::unopened
                    && !gameBoard[x - 1][y + 1].getHasMine()) {
                    if (gameBoard[x - 1][y + 1].getAdjacentNum() == 0) {
                        leftClick(x - 1, y + 1);
                    }
                    else {
                        gameBoard[x - 1][y + 1].setState(CellState::opened);
                    }
                }
                if (x + 1 < boardWidth && y + 1 < boardHeight
                    && gameBoard[x + 1][y + 1].getState() == CellState::unopened
                    && !gameBoard[x + 1][y + 1].getHasMine()) {
                    if (gameBoard[x + 1][y + 1].getAdjacentNum() == 0) {
                        leftClick(x + 1, y + 1);
                    }
                    else {
                        gameBoard[x + 1][y + 1].setState(CellState::opened);
                    }
                }
                if (x - 1 >= 0 && y - 1 >= 0
                    && gameBoard[x - 1][y - 1].getState() == CellState::unopened
                    && !gameBoard[x - 1][y - 1].getHasMine()) {
                    if (gameBoard[x - 1][y - 1].getAdjacentNum() == 0) {
                        leftClick(x - 1, y - 1);
                    }
                    else {
                        gameBoard[x - 1][y - 1].setState(CellState::opened);
                    }
                }
                if (x + 1 < boardWidth && y - 1 >= 0
                    && gameBoard[x + 1][y - 1].getState() == CellState::unopened
                    && !gameBoard[x + 1][y - 1].getHasMine()) {
                    if (gameBoard[x + 1][y - 1].getAdjacentNum() == 0) {
                        leftClick(x + 1, y - 1);
                    }
                    else {
                        gameBoard[x + 1][y - 1].setState(CellState::opened);
                    }
                }
            }
        }
    }
}

void rightClick(int x, int y) {
    if (status == GameStatus::alive) {
        if (gameBoard[x][y].getState() == CellState::flagged) {
            gameBoard[x][y].setState(CellState::unopened);
            flagCount += 1;
            draw();
        }
        else if (gameBoard[x][y].getState() == CellState::unopened && flagCount > 0) {
            gameBoard[x][y].setState(CellState::flagged);
            flagCount -= 1;
            draw();
        }
    }
}

void resetBtnAction() {
    if (status != GameStatus::alive) {
        initBoard();
    }
}

void draw() {
    SDL_SetRenderDrawColor(render, 221, 221, 221, 255);
    SDL_RenderClear(render);
    
    // draw borders
    SDL_SetRenderDrawColor(render, 204, 204, 204, 255);
    SDL_Rect r;
    r.x = 0; r.y = 0; r.w = 20 + (20 * boardWidth); r.h = 10;
    SDL_RenderFillRect(render, &r);
    r.x = 0; r.y = 50; r.w = 20 + (20 * boardWidth); r.h = 10;
    SDL_RenderFillRect(render, &r);
    r.x = 0; r.y = 60 + (20 * boardHeight); r.w = 20 + (20 * boardWidth); r.h = 10;
    SDL_RenderFillRect(render, &r);
    r.x = 0; r.y = 0; r.w = 10; r.h = 70 + (20 * boardHeight);
    SDL_RenderFillRect(render, &r);
    r.x = 10 + (20 * boardWidth); r.y = 0; r.w = 10; r.h = 70 + (20 * boardHeight);
    SDL_RenderFillRect(render, &r);
    
    SDL_Surface* btnImg;
    if (status == GameStatus::alive) {
        btnImg = SDL_LoadBMP("smilebtn.bmp");
    }
    else if (status == GameStatus::complete) {
        btnImg = SDL_LoadBMP("winbtn.bmp");
    }
    else {
        btnImg = SDL_LoadBMP("deadbtn.bmp");
    }
    SDL_Texture* btnTexture = SDL_CreateTextureFromSurface(render, btnImg);
    r.x = (20 * boardWidth)/2; r.y = 20; r.w = 20; r.h = 20;
    SDL_RenderCopy(render, btnTexture, NULL, &r);
    SDL_SetRenderDrawColor(render, 150, 150, 150, 255);
    SDL_RenderDrawRect(render, &r);

    r.x = 20; r.y = 20; r.w = 30; r.h = 20;
    SDL_RenderDrawRect(render, &r);
    SDL_SetRenderDrawColor(render, 204, 204, 204, 255);
    r.x = 21; r.y = 21; r.w = 28; r.h = 18;
    SDL_RenderFillRect(render, &r);
    string intermediary = to_string(flagCount);
    if (intermediary.length() == 1) {
        intermediary.insert(0, "0");
    }
    char * numString = const_cast<char*>(intermediary.c_str());
    renderText(numString, 25, 23, colorMap[7]);
    
    SDL_SetRenderDrawColor(render, 150, 150, 150, 255);
    r.x = 10; r.y = 10; r.w = 20 * boardWidth; r.h = 40;
    SDL_RenderDrawRect(render, &r);
    
    for (int i = 0; i <= boardWidth; i++) {
        SDL_RenderDrawLine(render, 10 + (i * 20), 60, 10 + (i * 20), 60 + (20 * boardHeight));
    }
    for (int i = 0; i <= boardHeight; i++) {
        SDL_RenderDrawLine(render, 10, 60 + (i * 20), 10 + (20 * boardWidth), 60 + (i * 20));
    }
    
    SDL_SetRenderDrawColor(render, 0, 255, 0, 255);
    r.x = ((20 * boardWidth) * 0.75) - 10; r.y = 25; r.w = 10; r.h = 10;
    SDL_RenderFillRect(render, &r);
    SDL_SetRenderDrawColor(render, 255, 255, 0, 255);
    r.x = ((20 * boardWidth) * 0.75) + 10; r.y = 25; r.w = 10; r.h = 10;
    SDL_RenderFillRect(render, &r);
    SDL_SetRenderDrawColor(render, 255, 0, 0, 255);
    r.x = ((20 * boardWidth) * 0.75) + 30; r.y = 25; r.w = 10; r.h = 10;
    SDL_RenderFillRect(render, &r);
    
    SDL_SetRenderDrawColor(render, 0, 0, 0, 255);
    switch (currentDifficulty) {
        case Difficulties::beginner:
            r.x = ((20 * boardWidth) * 0.75) - 11; break;
        case Difficulties::intermediate:
            r.x = ((20 * boardWidth) * 0.75) + 9; break;
        default:
            r.x = ((20 * boardWidth) * 0.75) + 29; break;
    }
    r.y = 24; r.w = 12; r.h = 12;
    SDL_RenderDrawRect(render, &r);
    
    for (int i = 0; i < boardWidth; i++) {
        for (int j = 0; j < boardHeight; j++) {
            if (gameBoard[i][j].getState() == CellState::unopened) {
                SDL_SetRenderDrawColor(render, 180, 180, 180, 255);
                r.x = 11 + (i * 20); r.y = 61 + (j * 20);
                r.w = 19; r.h = 19;
                SDL_RenderFillRect(render, &r);
            }
            else if (gameBoard[i][j].getState() == CellState::flagged) {
                SDL_Surface* flagImg = SDL_LoadBMP("flag.bmp");
                SDL_Texture* flagTexture = SDL_CreateTextureFromSurface(render, flagImg);
                r.x = 11 + (i * 20); r.y = 61 + (j * 20); r.w = 19; r.h = 19;
                SDL_RenderCopy(render, flagTexture, NULL, &r);
            }
            else {
                if (gameBoard[i][j].getAdjacentNum() != 0) {
                    int result = gameBoard[i][j].getAdjacentNum();
                    string intermediary = to_string(result);
                    char * numString = const_cast<char*>(intermediary.c_str());
                    int xCoord = result == 1 ? 17 + (i * 20) : 15 + (i * 20);
                    renderText(numString, xCoord, 63 + (j * 20), colorMap[result]);
                }
                else if (gameBoard[i][j].getHasMine()) {
                    SDL_Surface* mineImg;
                    if (i == highlightCoords[0] && j == highlightCoords[1]) {
                        mineImg = SDL_LoadBMP("mineHL.bmp");
                    }
                    else {
                        mineImg = SDL_LoadBMP("mine.bmp");
                    }
                    SDL_Texture* mineTexture = SDL_CreateTextureFromSurface(render, mineImg);
                    r.x = 11 + (i * 20); r.y = 61 + (j * 20); r.w = 19; r.h = 19;
                    SDL_RenderCopy(render, mineTexture, NULL, &r);
                }
            }
        }
    }
    
    SDL_RenderPresent(render);
}

void renderText(char *inpText, int x, int y, SDL_Color color) {
    TTF_Font *font = TTF_OpenFont("minesweeper.ttf", 12);
    SDL_Surface* textSurface = TTF_RenderText_Solid(font, inpText, color);
    SDL_Texture* text = SDL_CreateTextureFromSurface(render, textSurface);
    SDL_Rect r;
    r.x = x; r.y = y; r.w = textSurface->w; r.h = textSurface->h;
    SDL_RenderCopy(render, text, NULL, &r);
    SDL_DestroyTexture(text);
    SDL_FreeSurface(textSurface);
    TTF_CloseFont(font);
}

void close() {
    SDL_DestroyWindow(win);
    SDL_Quit();
}

int main() {
    init();
    render = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED);
    
    draw();
    bool isQuit = false;
    SDL_Event event;
    while (!isQuit) {
        if (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                isQuit = true;
            }
            else if (event.type == SDL_MOUSEBUTTONDOWN) {
                if (event.button.button == SDL_BUTTON_LEFT) {
                    if (event.motion.x >= ((20 * boardWidth) / 2) - 10
                        && event.motion.x <= ((20 * boardWidth) / 2) + 20
                        && event.motion.y >= 20 && event.motion.y <= 40) {
                        resetBtnAction();
                    }
                    else if (event.motion.x >= ((20 * boardWidth) * 0.75) - 10
                        && event.motion.x <= ((20 * boardWidth) * 0.75)
                        && event.motion.y >= 25 && event.motion.y <= 35
                        && currentDifficulty != Difficulties::beginner) {
                        currentDifficulty = Difficulties::beginner;
                        initBoard();
                    }
                    else if (event.motion.x >= ((20 * boardWidth) * 0.75) + 10
                        && event.motion.x <= ((20 * boardWidth) * 0.75) + 20
                        && event.motion.y >= 25 && event.motion.y <= 35
                        && currentDifficulty != Difficulties::intermediate) {
                        currentDifficulty = Difficulties::intermediate;
                        initBoard();
                    }
                    else if (event.motion.x >= ((20 * boardWidth) * 0.75) + 30
                        && event.motion.x <= ((20 * boardWidth) * 0.75) + 40
                        && event.motion.y >= 25 && event.motion.y <= 35
                        && currentDifficulty != Difficulties::expert) {
                        currentDifficulty = Difficulties::expert;
                        initBoard();
                    }
                    else {
                        leftClick(floor((event.motion.x - 10) / 20), floor((event.motion.y - 60) / 20));
                    }
                    draw();
                }
                else if (event.button.button == SDL_BUTTON_RIGHT) {
                    rightClick(floor((event.motion.x - 10) / 20), floor((event.motion.y - 60) / 20));
                }
                
                if (flagCount == 0) {
                    bool flag = true;
                    for (int i = 0; i < boardWidth; i++) {
                        for (int j = 0; j < boardHeight; j++) {
                            if ((gameBoard[i][j].getState() == CellState::unopened) ||
                                (gameBoard[i][j].getState() == CellState::flagged && !gameBoard[i][j].getHasMine())) {
                                flag = false;
                            }
                        }
                    }
                    if (flag) {
                        status = GameStatus::complete;
                        draw();
                    }
                }
            }
        }
    }
    
    close();
    
    return 0;
}