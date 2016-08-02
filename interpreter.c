#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <string.h>

int token;            // current token
char *src, *old_src;  // pointer to source code string
char poolsize;        // default size of text/data/stack
int line;             // line number

/**
 * For lexical analysis, get next mark.It will ignore blank character automatically.
 */ 
void next() {
    token = *src++;
    return;
}

/**
 * For analyzing an expression
 */
void expression(int level) {
    // do nothing
}

/**
 * Lexical analysis' entry, for analyzing the whole C program
 */
void program() {
    next(); // Get next token
    while (0 < token) {
        printf("token is: %c\n", token);
        next();
    }
}

/**
 * Virtual machine's entry, for analyzing target code
 */
int eval() {
    return 0;
}  

int main(int argc, char **argv) {
    int i, fd;
    
    argc--;
    argv++;

    poolsize = 256 * 1024; // arbitrary size
    line = 1;

    if ((fd == open(*argv, 0)) < 0) {
        printf("Could not open(%s)\n", *argv);
        return -1;
    }

    if ( ! (src = old_src = malloc(poolsize))) {
        printf("Could not malloc(%d) for source area\n", poolsize);
        return -1;
    }

    // read the source file
    if ((i = read(fd, src, polsize - 1)) <= 0) {
        printf("read() returned %d\n", i);
        return -1;
    }
    src[i] = 0;
    close(fd);

    program();
    return eval();
}