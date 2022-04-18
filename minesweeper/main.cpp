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
#include <sstream>
#include <filesystem>
#include <fstream>
#include "Cell.hpp"
using namespace std;

// Declare functions
void init();
void initBoard();
void setupHighScore();
void draw();
void close();
void renderText(char *inpText, int x, int y, SDL_Color color);
void leftClick(int x, int y);
void rightClick(int x, int y);
void resetBtnAction();
vector<Cell*> getNeighbours(int x, int y);
string secToTimeStamp(int input);
void relocateMine();
void labelCells();

// Window to render to
SDL_Window* win = NULL;
// Surface contained by win
SDL_Surface* surface = NULL;
// Renderer object responsible for graphics rendering
SDL_Renderer* render = NULL;

// Board dimensions
int boardHeight = 16;
int boardWidth = 30;

// Game board represented by vector of Cell objects
vector<vector<Cell>> gameBoard (boardWidth, vector<Cell>(boardHeight));

// For highlighting triggered mine
int highlightCoords[] = {-1, -1};

// For protecting first click
bool isFirstClick;

// Map of rgb values keyed by number of adjacent mines
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
// Map of width, height and number of mines keyed by corresponding difficulty
map <Difficulties, vector<int>> settingsMap = {
    {Difficulties::beginner, {9, 9, 10}},
    {Difficulties::intermediate, {16, 16, 40}},
    {Difficulties::expert, {30, 16, 99}}
};
Difficulties currentDifficulty = Difficulties::intermediate;

// Streams for reading and writing high score file
ofstream outStream;
ifstream inStream;
// Map of high scores keyed by difficulty
map<Difficulties, Uint64> highScores = {
    {Difficulties::beginner, -1},
    {Difficulties::intermediate, -1},
    {Difficulties::expert, -1}
};

// Current status of game
enum class GameStatus {alive, dead, complete};
GameStatus status = GameStatus::alive;

// Max number of flags available
int numFlags = 40;
// Number of clags used
int flagCount = numFlags;

// Value of last time SDL_GetTicks64() was called
Uint64 lastTime = 0;
// Offset from initialization of SDL library
Uint64 offset = 0;
// Current time from SDL_GetTicks64()
Uint64 currentTime = 0;

void init() {
    // Initialize SDL
    SDL_Init(SDL_INIT_VIDEO);
    // Initialize SDL_TTF (for font rendering)
    TTF_Init();
    // Create window
    win = SDL_CreateWindow("Minesweeper", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 340, 390, SDL_WINDOW_SHOWN);
    // Start first game
    initBoard();
}

// Read high scores into map from text file
void setupHighScore() {
    inStream.open("highscores.txt");
    
    string text;
    
    getline(inStream, text);
    if (text == "NA") { highScores[Difficulties::beginner] = -1; }
    else { highScores[Difficulties::beginner] = stoi(text); }
    
    getline(inStream, text);
    if (text == "NA") { highScores[Difficulties::intermediate] = -1; }
    else { highScores[Difficulties::intermediate] = stoi(text); }
    
    getline(inStream, text);
    if (text == "NA") { highScores[Difficulties::expert] = -1; }
    else { highScores[Difficulties::expert] = stoi(text); }
    
    inStream.close();
}

// Initialize game board and set up other prerequisites for game to start
void initBoard() {
    // First click is protected
    isFirstClick = true;
    
    // Get high scores
    setupHighScore();
    
    // Set board dimensions according to the current difficulty
    boardWidth = settingsMap[currentDifficulty][0];
    boardHeight = settingsMap[currentDifficulty][1];
    
    // Resize window according to board dimensions
    SDL_SetWindowSize(win, 20 + (20 * boardWidth), 95 + (20 * boardHeight));
    
    // Set number of flags according to the current difficulty
    numFlags = settingsMap[currentDifficulty][2];
    flagCount = numFlags;
    
    // Set game status to alive
    status = GameStatus::alive;
    
    // No mines triggered so no squares need to be highlighted (denoted by -1 coords)
    highlightCoords[0] = -1; highlightCoords[1] = -1;
    
    // Create board of blank cells ready for initialization
    for (int i = 0; i < boardWidth; i++) {
        for (int j = 0; j < boardHeight; j++) {
            gameBoard[i][j].setState(CellState::unopened);
            gameBoard[i][j].setHasMine(false);
            gameBoard[i][j].setAdjacentNum(0);
            gameBoard[i][j].setRow(i);
            gameBoard[i][j].setCol(j);
        }
    }
    
    // Randomly place mines in board
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
        // If cell already contains a mine, regenerate random coords
        else {
            i -= 1;
        }
    }
    
    // Label each cell with the number of adjacent mines
    labelCells();
    
    // Set offset to current time
    offset = currentTime;
    // Assign last time to curren time
    lastTime = floor(currentTime / 1000);
}

// Iterate through cells, labelling each with the number of adjacent mines
void labelCells() {
    for (int i = 0; i < boardWidth; i++) {
        for (int j = 0; j < boardHeight; j++) {
            int count = 0;
            if (!gameBoard[i][j].getHasMine()) {
                // Get current cell's neighbours
                vector<Cell*> neighbours = getNeighbours(i, j);
                
                for (Cell* c : neighbours) {
                    // If neighbour has mine, increment
                    if (c->getHasMine()) { count++; };
                }
                
                gameBoard[i][j].setAdjacentNum(count);
            }
        }
    }
}

// Convert milliseconds into timestamp string
string secToTimeStamp(int input) {
    // Use floor division to get number of each time denomination
    int hrs = floor(input / 3600);
    input -= (hrs * 3600);
    int mins = floor(input / 60);
    int secs = input - (mins * 60);
    
    string strHrs = to_string(hrs);
    string strMins = to_string(mins);
    string strSecs = to_string(secs);
    // Zero fill time denominations
    if (strHrs.length() == 1) { strHrs.insert(0, "0"); }
    if (strMins.length() == 1) { strMins.insert(0, "0"); }
    if (strSecs.length() == 1) { strSecs.insert(0, "0"); }
    
    // Format string
    if (hrs == 0) {
        return strMins + ":" + strSecs;
    }
    else {
        return strHrs + ":" + strMins + ":" + strSecs;
    }
}

// Return vector containing pointers to a cell's neighbours
vector<Cell*> getNeighbours(int x, int y) {
    vector<Cell*> result;
    
    if (x + 1 < boardWidth) {
        result.push_back(&gameBoard[x + 1][y]);
    }
    if (x - 1 >= 0) {
        result.push_back(&gameBoard[x - 1][y]);
    }
    if (y + 1 < boardHeight) {
        result.push_back(&gameBoard[x][y + 1]);
    }
    if (y - 1 >= 0) {
        result.push_back(&gameBoard[x][y - 1]);
    }
    if (x - 1 >= 0 && y + 1 < boardHeight) {
        result.push_back(&gameBoard[x - 1][y + 1]);
    }
    if (x + 1 < boardWidth && y + 1 < boardHeight) {
        result.push_back(&gameBoard[x + 1][y + 1]);
    }
    if (x - 1 >= 0 && y - 1 >= 0) {
        result.push_back(&gameBoard[x - 1][y - 1]);
    }
    if (x + 1 < boardWidth && y - 1 >= 0) {
        result.push_back(&gameBoard[x + 1][y - 1]);
    }
    
    return result;
}

// Function called when user left clicks
// Takes as parameters the coordinates for the cell clicked on
void leftClick(int x, int y) {
    // Check if click was within range of game board and was on a non-flagged cell
    if (x >= 0 && x < boardWidth && y >= 0 && y < boardHeight
        && gameBoard[x][y].getState() != CellState::flagged) {
        // Procedure for protecting first move
        if (isFirstClick && gameBoard[x][y].getHasMine()) {
            gameBoard[x][y].setHasMine(false);
            relocateMine();
            labelCells();
        }
        isFirstClick = false;
        
        if (status == GameStatus::alive) {
            // Open clicked on cell
            gameBoard[x][y].setState(CellState::opened);
            
            if (gameBoard[x][y].getHasMine()) {
                // If user clicked on mine, highlight that cell
                highlightCoords[0] = x; highlightCoords[1] = y;
                // Change status to dead
                status = GameStatus::dead;
                // Uncover all mines
                for (int i = 0; i < boardWidth; i++) {
                    for (int j = 0; j < boardHeight; j++) {
                        if (gameBoard[i][j].getHasMine()) {
                            gameBoard[i][j].setState(CellState::opened);
                        }
                    }
                }
            }
            
            // If clicked on cell has no adjacent mines uncover neighbours with no neighbouring mines
            // Continue recursively until reaching squares with adjacent mines
            else if (gameBoard[x][y].getAdjacentNum() == 0) {
                vector<Cell*> neighbours = getNeighbours(x, y);
                for (Cell* c : neighbours) {
                    if (c->getState() == CellState::unopened
                        && !c->getHasMine()) {
                        if (c->getAdjacentNum() == 0) {
                            leftClick(c->getRow(), c->getCol());
                        }
                        else {
                            c->setState(CellState::opened);
                        }
                    }
                }
            }
        }
    }
}

// Called if user triggers a mine on their first click
void relocateMine() {
    // Mine is moved to upper left hand corner
    // If upper left hand corner already has a mine, the mine moves to the right of the corner tile
    // Continues until mine finds a vacant cell
    for (int i = 0; i < boardWidth; i++) {
        for (int j = 0; j < boardHeight; j++) {
            if (!gameBoard[i][j].getHasMine()) {
                gameBoard[i][j].setHasMine(true);
                return;
            }
        }
    }
}

// Function called when user right clicks
// Used for placing flags
void rightClick(int x, int y) {
    if (status == GameStatus::alive) {
        // If cell is already flagged, remove flag and increment flagCount
        if (gameBoard[x][y].getState() == CellState::flagged) {
            gameBoard[x][y].setState(CellState::unopened);
            flagCount += 1;
            draw();
        }
        // If cell isn't flagged, add flag and decrement flagCount
        else if (gameBoard[x][y].getState() == CellState::unopened && flagCount > 0) {
            gameBoard[x][y].setState(CellState::flagged);
            flagCount -= 1;
            draw();
        }
    }
}

// Called when a game is lost or completed and user clicks top button
// Restarts game
void resetBtnAction() {
    if (status != GameStatus::alive) {
        initBoard();
    }
}

// Render game window
void draw() {
    SDL_SetRenderDrawColor(render, 221, 221, 221, 255);
    // Clear canvas
    SDL_RenderClear(render);
    
    // Draw borders
    SDL_SetRenderDrawColor(render, 204, 204, 204, 255);
    SDL_Rect r;
    r.x = 0; r.y = 0; r.w = 20 + (20 * boardWidth); r.h = 10;
    SDL_RenderFillRect(render, &r);
    r.x = 0; r.y = 50; r.w = 20 + (20 * boardWidth); r.h = 10;
    SDL_RenderFillRect(render, &r);
    r.x = 0; r.y = 60 + (20 * boardHeight); r.w = 20 + (20 * boardWidth); r.h = 35;
    SDL_RenderFillRect(render, &r);
    r.x = 0; r.y = 0; r.w = 10; r.h = 70 + (20 * boardHeight);
    SDL_RenderFillRect(render, &r);
    r.x = 10 + (20 * boardWidth); r.y = 0; r.w = 10; r.h = 70 + (20 * boardHeight);
    SDL_RenderFillRect(render, &r);
    
    // Draw top button
    // Image changes according to game status
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

    // Draw flag counter
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
    
    // Draw top bar outline
    SDL_SetRenderDrawColor(render, 150, 150, 150, 255);
    r.x = 10; r.y = 10; r.w = 20 * boardWidth; r.h = 40;
    SDL_RenderDrawRect(render, &r);
    // Draw grid lines
    for (int i = 0; i <= boardWidth; i++) {
        SDL_RenderDrawLine(render, 10 + (i * 20), 60, 10 + (i * 20), 60 + (20 * boardHeight));
    }
    for (int i = 0; i <= boardHeight; i++) {
        SDL_RenderDrawLine(render, 10, 60 + (i * 20), 10 + (20 * boardWidth), 60 + (i * 20));
    }
    
    // Draw difficulty buttons
    SDL_SetRenderDrawColor(render, 0, 255, 0, 255);
    r.x = ((20 * boardWidth) * 0.75) - 10; r.y = 25; r.w = 10; r.h = 10;
    SDL_RenderFillRect(render, &r);
    SDL_SetRenderDrawColor(render, 255, 255, 0, 255);
    r.x = ((20 * boardWidth) * 0.75) + 10; r.y = 25; r.w = 10; r.h = 10;
    SDL_RenderFillRect(render, &r);
    SDL_SetRenderDrawColor(render, 255, 0, 0, 255);
    r.x = ((20 * boardWidth) * 0.75) + 30; r.y = 25; r.w = 10; r.h = 10;
    SDL_RenderFillRect(render, &r);
    
    // Draw red border showing selected difficulty
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
    
    // Iterate through cells and render
    for (int i = 0; i < boardWidth; i++) {
        for (int j = 0; j < boardHeight; j++) {
            // If cell is unopened, draw grey square
            if (gameBoard[i][j].getState() == CellState::unopened) {
                SDL_SetRenderDrawColor(render, 180, 180, 180, 255);
                r.x = 11 + (i * 20); r.y = 61 + (j * 20);
                r.w = 19; r.h = 19;
                SDL_RenderFillRect(render, &r);
            }
            // If cell is flagged, draw flag sprite
            else if (gameBoard[i][j].getState() == CellState::flagged) {
                SDL_Surface* flagImg = SDL_LoadBMP("flag.bmp");
                SDL_Texture* flagTexture = SDL_CreateTextureFromSurface(render, flagImg);
                r.x = 11 + (i * 20); r.y = 61 + (j * 20); r.w = 19; r.h = 19;
                SDL_RenderCopy(render, flagTexture, NULL, &r);
            }
            else {
                // If cell has neighbouring mines, render number of adjacent mines
                if (gameBoard[i][j].getAdjacentNum() != 0) {
                    int result = gameBoard[i][j].getAdjacentNum();
                    string intermediary = to_string(result);
                    char * numString = const_cast<char*>(intermediary.c_str());
                    int xCoord = result == 1 ? 17 + (i * 20) : 15 + (i * 20);
                    renderText(numString, xCoord, 63 + (j * 20), colorMap[result]);
                }
                // If cell contains a mine, draw mine sprite
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
    
    // Draw timer
    intermediary = secToTimeStamp(floor((SDL_GetTicks64() - offset) / 1000));
    char * t = intermediary.data();
    renderText(t, 10, 70 + (20 * boardHeight), colorMap[7]);
    
    // Draw best time if it exists
    if (highScores[currentDifficulty] != -1) {
        intermediary = secToTimeStamp(floor(highScores[currentDifficulty] / 1000));
        intermediary.insert(0, "Best Time: ");
        char * t = intermediary.data();
        renderText(t, -1, 70 + (20 * boardHeight), colorMap[7]);
    }
    
    SDL_RenderPresent(render);
}

// Function for rendering text
void renderText(char *inpText, int x, int y, SDL_Color color) {
    TTF_Font *font = TTF_OpenFont("minesweeper.ttf", 12);
    SDL_Surface* textSurface = TTF_RenderText_Solid(font, inpText, color);
    SDL_Texture* text = SDL_CreateTextureFromSurface(render, textSurface);
    SDL_Rect r;
    r.x = x; r.y = y; r.w = textSurface->w; r.h = textSurface->h;
    // If -1 is passed in for x, draw from right hand side of window using textSurfce->w
    if (x == -1) {
        r.x = ((boardWidth * 20) + 10) - textSurface->w;
    }
    SDL_RenderCopy(render, text, NULL, &r);
    // Free up resources
    SDL_DestroyTexture(text);
    SDL_FreeSurface(textSurface);
    TTF_CloseFont(font);
}

// Free resources and close SDL + SDL_TTF
void close() {
    SDL_DestroyWindow(win);
    TTF_Quit();
    SDL_Quit();
}

// Main game loop
int main() {
    //Initialize SDL
    init();
    // Create renderer
    render = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED);
    
    // Render first frame
    draw();
    // Quit flag
    bool isQuit = false;
    // Poll event
    SDL_Event event;
    while (!isQuit) {
        // Capture current time elapsed since intialization of SDL
        currentTime = SDL_GetTicks64();
        // Re-render window if a second has elapsed since lastTime (updates timer)
        if (status == GameStatus::alive && currentTime > lastTime + 1000) {
            draw();
            lastTime = currentTime;
        }
        
        // Event handling
        if (SDL_PollEvent(&event)) {
            // Break loop if quit
            if (event.type == SDL_QUIT) {
                isQuit = true;
            }
            // Handle muse click
            else if (event.type == SDL_MOUSEBUTTONDOWN) {
                // Handle left mouse click
                if (event.button.button == SDL_BUTTON_LEFT) {
                    // If top button is clicked, call relevant function
                    if (event.motion.x >= ((20 * boardWidth) / 2) - 10
                        && event.motion.x <= ((20 * boardWidth) / 2) + 20
                        && event.motion.y >= 20 && event.motion.y <= 40) {
                        resetBtnAction();
                    }
                    // If difficulty button clicked, adjust currentDiffulty accordingly
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
                    // Otherwise, call usual leftClick
                    else {
                        leftClick(floor((event.motion.x - 10) / 20), floor((event.motion.y - 60) / 20));
                    }
                    // Re-render window
                    draw();
                }
                // Handle right mouse click
                else if (event.button.button == SDL_BUTTON_RIGHT) {
                    rightClick(floor((event.motion.x - 10) / 20), floor((event.motion.y - 60) / 20));
                }
                
                // Check for game completion
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
                        // If new best time achieved, save to text file
                        if (highScores[currentDifficulty] == -1
                            || SDL_GetTicks64() - offset < highScores[currentDifficulty]) {
                            highScores[currentDifficulty] = SDL_GetTicks64() - offset;
                            outStream.open("highscores.txt", ofstream::out | ofstream::trunc);
                            for (auto& [key, value] : highScores) {
                                if (value == -1) {
                                    outStream << "NA\n";
                                }
                                else {
                                    outStream << (to_string(value) + "\n");
                                }
                            }
                            outStream.close();
                        }
                        draw();
                    }
                }
            }
        }
    }
    
    // Quit SDL if while loop is broken
    close();
    
    return 0;
}
