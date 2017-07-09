#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <sys/select.h>

#include "terminal.h"

struct termios _originalTermios;


void reset_terminal_mode(void) {
    tcsetattr(0, TCSANOW, &_originalTermios);
}

void set_conio_terminal_mode(void) {
    struct termios newTermios;

    /* take two copies - one for now, one for later */
    tcgetattr(0, &_originalTermios);
    memcpy(&newTermios, &_originalTermios, sizeof(newTermios));

    /* register cleanup handler, and set the new terminal mode */
    atexit(reset_terminal_mode);
    cfmakeraw(&newTermios);
    tcsetattr(0, TCSANOW, &newTermios);
}

int kbhit(void) {
    struct timeval tv = { 0L, 0L };
    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(0, &fds);
    return select(1, &fds, NULL, NULL, &tv);
}

int getch(void) {
    int r;
    unsigned char c;
    if ((r = read(0, &c, sizeof(c))) < 0) {
        return r;
    } else {
        return c;
    }
}
