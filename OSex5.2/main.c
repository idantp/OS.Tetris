// Author: Idan Twito
// ID: 311125249
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

#define BUFFER_SIZE 1
#define MOVE_LEFT 'a'
#define MOVE_DOWN 's'
#define MOVE_RIGHT 'd'
#define FLIP_SHAPE 'w'
#define QUIT_GAME 'q'
#define MATRIX_SIZE 20
#define MATRIX_CENTER_X 10
#define MATRIX_START_Y 0
#define HYPHEN '-'
#define SPACE_CHAR ' '
#define STAR_CHAR '*'
#define NEW_LINE "\n"
#define ONE_SECOND 1
#define NOT_ALARM_SIGNAL 0
#define SCREEN_SIZE 20
#define LAST_DISPLAY_LINE 19


/**
 * the display of the game
 */
typedef struct TetrisDisplays {
    char background[MATRIX_SIZE][MATRIX_SIZE];
} TetrisDisplay;

/**
 * the shape of the game
 */
typedef struct Shapes {
    bool isStanding;
    // indicates the position of the middle square("-")
    int middleSquarePosX;
    int middleSquarePosY;
} Shape;

// describes the Game's display
TetrisDisplay display;
// describes the shape in the game
Shape threeSquares;

/**
 * Function Name: threeSquaresInit
 * @param shape - the game's shape we'd like to initialize
 * Function Operation: the function sets the shape's position
 */
void threeSquaresInit(Shape *shape) {
    shape->isStanding = false;
    shape->middleSquarePosX = MATRIX_CENTER_X;
    shape->middleSquarePosY = MATRIX_START_Y;
}

/**
 * Function Name: printGame
 * Function Operation: the function prints the matrix which describes the background and the
 *                     shape inside of it.
 */
void printGame() {
    system("clear");
    int line, column;
    for (line = 0; line < MATRIX_SIZE; line++) {
        for (column = 0; column < MATRIX_SIZE; column++) {
            printf("%c", display.background[line][column]);
        }
        printf(NEW_LINE);
    }
}

/**
 * Function Name: layingThreeSquaresSet
 * Function Operation: the function sets the laying shape's squares on the display
 */
void layingThreeSquaresSet() {
    int middleXPos = threeSquares.middleSquarePosX;
    int middleYPos = threeSquares.middleSquarePosY;
    int leftPos = middleXPos - 1;
    int rightPos = middleXPos + 1;
    display.background[middleYPos][leftPos] = HYPHEN;
    display.background[middleYPos][middleXPos] = HYPHEN;
    display.background[middleYPos][rightPos] = HYPHEN;
}

/**
 * Function Name: standingThreeSquaresSet
 * Function Operation: the function sets the standing shape's squares on the display
 */
void standingThreeSquaresSet() {
    int middlePos = threeSquares.middleSquarePosX;
    int yCoordinate = threeSquares.middleSquarePosY;
    int upperPos = yCoordinate - 1;
    int lowerPos = yCoordinate + 1;
    display.background[upperPos][middlePos] = HYPHEN;
    display.background[yCoordinate][middlePos] = HYPHEN;
    display.background[lowerPos][middlePos] = HYPHEN;
}

/**
 * Function Name: shapeEraser
 * Function Operation: the function erases the shape from the matrix
 */
void shapeEraser() {
    if (threeSquares.isStanding) {
        display.background[threeSquares.middleSquarePosY][threeSquares.middleSquarePosX] = SPACE_CHAR;
        display.background[threeSquares.middleSquarePosY -
                           1][threeSquares.middleSquarePosX] = SPACE_CHAR;
        display.background[threeSquares.middleSquarePosY +
                           1][threeSquares.middleSquarePosX] = SPACE_CHAR;
    } else {
        display.background[threeSquares.middleSquarePosY][threeSquares.middleSquarePosX] = SPACE_CHAR;
        display.background[threeSquares.middleSquarePosY][threeSquares.middleSquarePosX -
                                                          1] = SPACE_CHAR;
        display.background[threeSquares.middleSquarePosY][threeSquares.middleSquarePosX +
                                                          1] = SPACE_CHAR;
    }
}

/**
 * Function Name: shapeDropper
 * @param sig describes the signal which was sent:
 *           SIGALRM - means the a second has passed - the function drops the shape automatically
 *           0       - means the player has pressed "MOVE_DOWN" - the function drops the shape
 * Function Operation:
 */
void shapeDropper(int sig) {
    // if called by ALARM SIGNAL - set the alarm for 1 second
    if (sig == SIGALRM) {
        signal(SIGALRM, shapeDropper);
        alarm(ONE_SECOND);
    }
    // if dropping is possible - drop the shape
    if ((!(threeSquares.isStanding) && threeSquares.middleSquarePosY <= (SCREEN_SIZE - 3)) ||
        ((threeSquares.isStanding) && threeSquares.middleSquarePosY <= (SCREEN_SIZE - 4))) {
        shapeEraser();
        threeSquares.middleSquarePosY++;
        if (threeSquares.isStanding) {
            standingThreeSquaresSet();
        } else {
            layingThreeSquaresSet();
        }
        printGame();
        // dropping the shape is not possible once the shape reaches the bottom line, in this
        // case the function draws the shape in the top of the display
    } else if ((!(threeSquares.isStanding) && threeSquares.middleSquarePosY == (SCREEN_SIZE - 2)) ||
               ((threeSquares.isStanding) && threeSquares.middleSquarePosY == (SCREEN_SIZE - 3))) {
        shapeEraser();
        threeSquaresInit(&threeSquares);
        layingThreeSquaresSet();
        printGame();
    }
}

/**
 * Function Name: shapeMoveRight
 * Function Operation: the function moves the shape to the right if possible.
 */
void shapeMoveRight() {
    if ((!(threeSquares.isStanding) && threeSquares.middleSquarePosX <= (SCREEN_SIZE - 4)) ||
        ((threeSquares.isStanding) && threeSquares.middleSquarePosX <= (SCREEN_SIZE - 3))) {
        shapeEraser();
        threeSquares.middleSquarePosX++;
        if (threeSquares.isStanding) {
            standingThreeSquaresSet();
        } else {
            layingThreeSquaresSet();
        }
        printGame();
    }
}

/**
 * Function Name: shapeMoveLeft
 * Function Operation: the function moves the shape to the left if possible.
 */
void shapeMoveLeft() {
    if ((!(threeSquares.isStanding) && threeSquares.middleSquarePosX >= (SCREEN_SIZE - 17)) ||
        ((threeSquares.isStanding) && threeSquares.middleSquarePosX >= (SCREEN_SIZE - 18))) {
        shapeEraser();
        threeSquares.middleSquarePosX--;
        if (threeSquares.isStanding) {
            standingThreeSquaresSet();
        } else {
            layingThreeSquaresSet();
        }
        printGame();
    }
}

/**
 * Function Name: flipShape
 * Function Operation: the function flips the shape once 'w' is pressed
 */
void flipShape() {
    if (!threeSquares.isStanding && threeSquares.middleSquarePosY <= (SCREEN_SIZE - 3) &&
        threeSquares.middleSquarePosY >= (SCREEN_SIZE - 19)) {
        shapeEraser();
        threeSquares.isStanding = true;
        standingThreeSquaresSet();
        printGame();
        return;
    } else if (threeSquares.isStanding && threeSquares.middleSquarePosX >= (SCREEN_SIZE - 18) &&
               threeSquares.middleSquarePosX <= (SCREEN_SIZE - 3)) {
        shapeEraser();
        threeSquares.isStanding = false;
        layingThreeSquaresSet();
        printGame();
    }
}

/**
 * Function Name: backGroundInit
 * @param display - the game's matrix display
 * Function Operation: the function draws the border of the background in stars (*)
 */
void backGroundInit(char display[MATRIX_SIZE][MATRIX_SIZE]) {
    int i, j;
    for (i = 0; i < MATRIX_SIZE; i++) {
        for (j = 0; j < MATRIX_SIZE; j++) {
            if ((i != LAST_DISPLAY_LINE && (j == 0 || j == LAST_DISPLAY_LINE)) ||
                i == LAST_DISPLAY_LINE) {
                display[i][j] = STAR_CHAR;
            } else { display[i][j] = SPACE_CHAR; }
        }
    }
}

/**
 * Function Name: sigUsrHandler
 * @param sig the signal which is sent
 * Function Operation: the function is called once one of the game's button are pressed.
 *                     the function moves the shape accordingly, or flips the shape, or exits
 *                     the game
 */
void sigUsrHandler(int sig) {
    signal(SIGUSR2, sigUsrHandler);
    char buffer[1];
    // read the char which was sent (the reading is from the pipe - this was set in ex51.c)
    read(0, buffer, BUFFER_SIZE);
    char c = buffer[0];
    switch (c) {
        case MOVE_LEFT:
            shapeMoveLeft();
            break;
        case MOVE_RIGHT:
            shapeMoveRight();
            break;
        case MOVE_DOWN:
            shapeDropper(NOT_ALARM_SIGNAL);
            break;
        case FLIP_SHAPE:
            flipShape();
            break;
        case QUIT_GAME:
            exit(0);
        default:
            break;
    }

}

int main() {
    // initializing the background matrix
    backGroundInit(display.background);
    threeSquaresInit(&threeSquares);
    layingThreeSquaresSet();
    printGame();
    // every second shapeDropper drops the shape by one line
    signal(SIGALRM, shapeDropper);
    // once the game's buttons are pressed sigUsrHandler func handles the game accordingly
    signal(SIGUSR2, sigUsrHandler);
    // sets the alarm for one second
    alarm(ONE_SECOND);
    while (1) {
        //prevents busy waiting
        pause();
    }
    return 0;
}