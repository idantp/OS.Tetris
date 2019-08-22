// Author: Idan Twito
// ID: 311125249
#include <stdio.h>
#include <unistd.h>
#include <termios.h>
#include <string.h>
#include <signal.h>

#define ERROR_MSG "Error in system call\n"
#define DRAW_OUT_FILE "./draw.out"
#define FILE_DESC 2
#define MOVE_LEFT 'a'
#define MOVE_DOWN 's'
#define MOVE_RIGHT 'd'
#define FLIP_SHAPE 'w'
#define QUIT_GAME 'q'
#define ERROR_RETURN 1
#define FUNCTION_ERROR -1
#define BYTE_SIZE 1
#define WRITE_TO_PIPE 1
#define READ_FROM_PIPE 0

char getch() {
    char buf = 0;
    struct termios old = {0};
    if (tcgetattr(0, &old) < 0)
        perror("tcsetattr()");
    old.c_lflag &= ~ICANON;
    old.c_lflag &= ~ECHO;
    old.c_cc[VMIN] = 1;
    old.c_cc[VTIME] = 0;
    if (tcsetattr(0, TCSANOW, &old) < 0)
        perror("tcsetattr ICANON");
    if (read(0, &buf, 1) < 0)
        perror("read()");
    old.c_lflag |= ICANON;
    old.c_lflag |= ECHO;
    if (tcsetattr(0, TCSADRAIN, &old) < 0)
        perror("tcsetattr ~ICANON");
    return (buf);
}

/**
 * Function Name: errorPrint
 * Function Input: char *errorMsg
 * Function Output: void
 * Function Operation: writes the error message which errorMsg contains
 */
void errorPrint(char *errorMsg) {
    size_t size = strlen(errorMsg);
    write(FILE_DESC, errorMsg, size);
}

/**
 * Function Name: pipeCreator
 * @param fdaArg serves as a pipe
 * @return the return value of pipe() operation
 */
int pipeCreator(int fdaArg[2]) {
    return pipe(fdaArg);
}

/**
 * Function Name: sendUSR2
 * @param pid the process ID to which the signal is sent
 */
void sendUSR2(pid_t pid) {
    kill(pid, SIGUSR2);
}

/**
 * Function Name: writeLetterToPipe
 * @param c - the function writes the given char in the pipe that is given
 * @param pipeFd - the pipe to which the function writes the char
 * Function Operation: the function write the given char into the given pipe
 */
void writeLetterToPipe(char c, int pipeFd[2]) {
    char buffer[1];
    buffer[0] = c;
    int res = write(pipeFd[WRITE_TO_PIPE], buffer, BYTE_SIZE);
    if (res == FUNCTION_ERROR) { errorPrint("ERROR IN WRITE\n"); }
}

/**
 * Function Name: buttonHandler
 * @param pid the process ID of the process to which the function writes to through the pipe
 * @param pipeFd this pipe serves as a communication between the father process and his child
 * Function Operation: the function reads the keyboard's keys that are pressed by the user, and
 *                     writes to the pipe those are useful to the Tetris game,
 *
 */
void buttonHandler(pid_t pid, int pipeFd[2]) {
    char c = ' ';
    while (c != QUIT_GAME) {
        // read a new key
        c = getch();
        // if a useful key is pressed - send a signal to the given process and write the button
        // to the pipe (so the Tetris can handle this button)
        switch (c) {
            case MOVE_LEFT:
                sendUSR2(pid);
                writeLetterToPipe(MOVE_LEFT, pipeFd);
                break;
            case MOVE_DOWN:
                sendUSR2(pid);
                writeLetterToPipe(MOVE_DOWN, pipeFd);
                break;
            case MOVE_RIGHT:
                sendUSR2(pid);
                writeLetterToPipe(MOVE_RIGHT, pipeFd);
                break;
            case FLIP_SHAPE:
                sendUSR2(pid);
                writeLetterToPipe(FLIP_SHAPE, pipeFd);
                break;
            default:
                continue;
        }
    }
    // if 'q' is pressed then the game is over
    sendUSR2(pid);
    writeLetterToPipe(QUIT_GAME, pipeFd);
}

int main() {
    // pipe for a communication between this project to the Tetris project
    int pipeFd[2];
    pid_t pid;
    // create a new pipe
    if (pipeCreator(pipeFd) == FUNCTION_ERROR) {
        errorPrint(ERROR_MSG);
        return ERROR_RETURN;
    }
    // create a new process
    pid = fork();
    if (pid > 0) {
        // father process - responsible for writing the buttons that are pressed in the pipe
        // declaring the default writer file descriptor as the pipe's writer side
        dup2(pipeFd[WRITE_TO_PIPE], 1);
        // close the pipe's reading file descriptor
        close(pipeFd[READ_FROM_PIPE]);
        buttonHandler(pid, pipeFd);
        // this line happens when the QUIT_GAME button is pressed
        close(pipeFd[WRITE_TO_PIPE]);
    } else if (pid == 0) {
        //child process - responsible for executing the Tetris game and for making opening the
        // reading file descriptor in the pipe as the default reading file descriptor
        dup2(pipeFd[READ_FROM_PIPE], 0);
        // close writing file descriptor
        close(pipeFd[WRITE_TO_PIPE]);
        // execute Tetris Game
        char *runCommand[] = {DRAW_OUT_FILE, NULL};
        if (execvp(runCommand[0], runCommand) == FUNCTION_ERROR) {
            close(pipeFd[READ_FROM_PIPE]);
            errorPrint(ERROR_MSG);
            return ERROR_RETURN;
        }
    } else {
        errorPrint(ERROR_MSG);
        close(pipeFd[READ_FROM_PIPE]);
        close(pipeFd[WRITE_TO_PIPE]);
        return ERROR_RETURN;
    }
    return 0;
}