/*
 * Tetris written in ncurses:
 * 1. man ncurses
 * 2. less ncurses.h
 */

#include <stdlib.h>
#include <locale.h>
#include <string.h>
#include <assert.h>
//#include <time.h>
#include <unistd.h> // POSIX standard C libary: sleep(), usleep()
#include <ncurses.h> // TUI, graphics

/* wchar_t tetromino[7]; */
// 4x4 tetromino = 16 without '\n' + '\0' = 17
const char tetrominos[7][17] = {
    "..X."
    "..X."
    "..X."
    "..X.",

    "..X."
    ".XX."
    ".X.."
    "....",

    ".X.."
    ".XX."
    "..X."
    "....",

    "...."
    ".XX."
    ".XX."
    "....",

    "...."
    ".XX."
    "..X."
    "..X.",

    "...."
    ".XX."
    ".X.."
    ".X..",

    ".X.."
    ".XX."
    ".X.."
    "...."
};

int fieldWidth = 12;
int fieldHeight = 18;

// unsigned char = byte
// dynamically allocated field of tetrominos
unsigned char* field = NULL;

// Possibly broken

// Translated (px, py) inside tetromino according 
// to rotation to the rotated index

// Use only for TETROMINOS
static int posToIndex(int px, int py, int r) {
    switch (r % 4) {
        case 0: return px + py * 4;        // 0º
        case 1: return 12 + py - (px * 4); // 90º
        case 2: return 15 - (py * 4) - px; // 180º
        case 3: return 3 - py + (px * 4);  // 270º
    }
    return 0;
}

// Checks whether tetromino (id) with rotation (r)
// intersects point (px, py) in the field
static bool doesnotCollide(int tetrominoId, int r, int px, int py) {
    // Iterate tetromino
    for (int y = 0; y < 4; y++) {
        for (int x = 0; x < 4; x++) {
            int i = posToIndex(x, y, r);
            int fieldIndex = (px + x) + (py + y) * fieldWidth;

            // Check bounds
            if (py + y >= 0 && py + y < fieldHeight) {
                if (px + x >= 0 && px + x < fieldWidth) {
                    // Check for collision tetromino (X) and field (!0)
                    if (tetrominos[tetrominoId][i] == 'X' && field[fieldIndex] != 0) {
                        return false;
                    }
                }
            }
        }
    }

    return true;
}

static void gameOverAnimation() {
}

int main(void) {
    // Allocate block of memory for the field
    field = (unsigned char*)malloc(fieldWidth * fieldHeight * sizeof(unsigned char));
    if (field == NULL) {
        addstr("Error, allocating block of memory");
        getch();
        endwin();
        return EXIT_FAILURE;
    }

    // Initialize tetris field
    for (int y = 0; y < fieldHeight; y++) {
        for (int x = 0; x < fieldWidth; x++) {
            if (x == 0 || x == fieldWidth - 1 || y == fieldHeight - 1)
                field[x + y * fieldWidth] = 9; // bounds
            else
                field[x + y * fieldWidth] = 0; // empty
        }
    }

    // Init ncurses
    setlocale(LC_ALL, "");
    initscr();

    // Configure ncurses screen
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    halfdelay(1); // half-delay mode for 0.1 second

    int maxlines = LINES;
    int maxcols = COLS;

    bool gameOver = false;
    int score = 0;

    /* struct { */
    /* } currentPiece; */

    // Tetromino state
    int currentPieceIndex = 1; // tetromino id=index inside tetrominos array
    int currentPieceRotation = 0;
    // Global, field tetromino position
    int currentPieceX = fieldWidth / 2; // x offset
    int currentPieceY = 0; // y offset

    int speed = 10; // 1.5s, 20 game ticks = 3 seconds
    int speedCount = 0; // ticks count

    // 0.1 for user input + 0.05 sleep time = 0.15 seconds + execution time
    // game tick = 0.15 seconds + execution
    while (!gameOver) {
        // Time

        //
        // Input:
        //
        int ch = 0;
        ch = getch(); // get user key pressed

        if (ch == (int)'q') {
            break;
        }

        // to get asynchronous input from the console
        // requires C/C++ multithreading knowledge
        // or get halt input (only 1 thread)

        //
        // Update (logic):
        //
        if (ch == KEY_LEFT) {
            if (doesnotCollide(currentPieceIndex, currentPieceRotation, currentPieceX - 1, currentPieceY)) {
                // Move left
                currentPieceX -= 1;
            }
        } else if (ch == KEY_RIGHT) {
            if (doesnotCollide(currentPieceIndex, currentPieceRotation, currentPieceX + 1, currentPieceY)) {
                // Move right
                currentPieceX += 1;
            }
        } else if (ch == KEY_DOWN) {
            if (doesnotCollide(currentPieceIndex, currentPieceRotation, currentPieceX, currentPieceY + 1)) {
                // Move down
                currentPieceY += 1;
            }
        } else if (ch == (int)'z') {
            if (doesnotCollide(currentPieceIndex, currentPieceRotation + 1, currentPieceX, currentPieceY)) {
                // Rotate 90º clockwise
                currentPieceRotation += 1;
            }
        }

        // Increment speed from 0 to 20
        speedCount = (speedCount + 1) % 21;

        // Move piece down every time
        if (speedCount >= speed) {
            if (doesnotCollide(currentPieceIndex, currentPieceRotation, currentPieceX, currentPieceY + 1)) {
                // Move down
                currentPieceY += 1;
            } else {
                // Piece fell down or collided

                // Whether game over??
                if (currentPieceY == 0) {
                    // Game over
                    break;
                }

                // Save piece in the field
                // iterate over piece, save individual cells
                for (int x = 0; x < 4; x++) {
                    for (int y = 0; y < 4; y++) {
                        // Index inside tetromino
                        int i = posToIndex(x, y, currentPieceRotation);

                        // Lock piece in the field
                        if (tetrominos[currentPieceIndex][i] == 'X') {
                            int fi = currentPieceX + x +
                                (currentPieceY + y) * fieldWidth;
                            //int c = (char)(65 + currentPieceIndex);
                            //char c = " ABCDEFG=#"[field[x + y * fieldWidth]];

                            // Save piece id
                            field[fi] = currentPieceIndex + 1;
                        }
                    }
                }

                // Get next piece
                currentPieceIndex = (currentPieceIndex + 1) % 7;

                // Reset stats
                currentPieceRotation = 0;
                currentPieceX = fieldWidth / 2;
                currentPieceY = 0;

                // Check tetromino lines completion

                // Tetromino:
                //     x
                //   +----->
                // y | XX  
                //   | XX  
                //   |     
                //   v

                // Field:
                //     y
                //   +---------->
                // x |           
                //   | AAA       
                //   |  A        
                //   v           

                // Iterate field (excluding bounds, -1)
                for (int fx = 0; fx < fieldHeight - 1; fx++) {
                    bool isTetrominoLine = true;
                    for (int fy = 1; fy < fieldWidth - 1; fy++) {
                        int fi = fy + fx * fieldWidth;

                        // Now line already if empty
                        if (field[fi] == 0) {
                            isTetrominoLine = false;
                            break;
                        }
                    }

                    if (isTetrominoLine) {
                        // Increment score
                        score += 1;

                        // Remove tetromino line
                        for (int fy = 1; fy < fieldWidth - 1; fy++) {
                            int fi = fy + fx * fieldWidth;
                            field[fi] = 0;
                        }

                        // Move all upper pieces down
                        // Iterate lines from the tetromino line to the up
                        for (int fx2 = fx - 1; fx2 >= 0; fx2--) {
                            // Iterate cells
                            for (int fy2 = 1; fy2 < fieldWidth - 1; fy2++) {
                                int fi = fy2 + fx2 * fieldWidth;
                                int fi2 = fy2 + (fx2 + 1) * fieldWidth;
                                // Move cell down
                                field[fi2] = field[fi];

                                // Remove cell
                                field[fi] = 0;
                            }
                        }

                        // Check for the updated lines again from the start
                        fx = 0;

                        // TODO: line completion animation
                    }
                }
            }
        }

        //
        // Render:
        //

        clear();

        // Screen:
        //     x
        //   +--------------->
        // y |   12    score 
        //   |  Field:       
        //   | 1 tetr  next  
        //   v 8             

        // Draw field
        for (int y = 0; y < fieldHeight; y++) {
            for (int x = 0; x < fieldWidth; x++) {
                // ' ' - nothing // A-G - tetromino // =, # - boundaries
                char c = " ABCDEFG=#"[field[x + y * fieldWidth]];
                mvaddch(y, x, c);
            }
        }

        // Draw current tetromino piece
        for (int y = 0; y < 4; y++) {
            for (int x = 0; x < 4; x++) {
                int i = posToIndex(x, y, currentPieceRotation);
                if (tetrominos[currentPieceIndex][i] == 'X') {
                    char c = (char)65 + currentPieceIndex;
                    mvaddch(y + currentPieceY, x + currentPieceX, c);
                }
            }
        }

        // Draw score
        int scorePosX = 16;
        int scorePosY = 1;
        mvputs(scorePosY, scorePosX, strcat("Score:", itos(score))

        // Draw next tetromino
        int nextPosX = 0;
        int nextPosY = 0;

        refresh();

        // Game tick
        usleep(50 * 100); // 50ms = 0.05 seconds
    }

    refresh();
    getch();
    free(field); // free field resources
    endwin();
    return EXIT_SUCCESS;
}
