#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

int token;            // current token
char *src, *old_src;  // pointer to source code string
int poolsize;        // default size of text/data/stack
int line;             // line number

/**
 * Memory segment
 */
int *text; // text segment, to store code 
int *old_text; // for dump text segment
int *stack; // stack segment
char *data; // segment, for initialized data(string)

/**
 * Virtual machine registera
 */
int *pc; // program counter, stores a memory address, which stores next conputer command to execute
int *bp;  // base address pointer, point to some place of the stack.
int *sp; // pointer register, point to the top of the stack.Address is from high to low, sp will decriminate when pushing to the stack
int ax; // common register, store the result of one executed command
int cycle;

/** 
 * CPU command set
 */
enum {LEA, IMM, JMP, CALL, JZ, JNZ, ENT, ADJ, LEV, LI, LC, SI, SC, PUSH, OR, XOR, AND, EQ, NE, LT, GT, LE, GE,
        SHL, SHR, ADD, SUB, MUL, DIV, MOD, OPEN, READ, CLOS, PRTF, MALC, MSET, MCMP, EXIT}; 


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
    int op;
    int *tmp;
    while (1) {
        /**
         * IMM<num> is to put <num> into register ax,
         * LC is to put the corresbonding address' character into ax, requiring ax stores address,
         * LI is to put the corresbonding address' integer into ax, requiring ax stores address,
         * SC puts the data in ax into the address as a character, requiring the top of the stack stores address,
         * SI puts the data in ax into the address as an integer, requiring the top of the stack stores address
         */
        if(op == IMM) {
            ax = *pc++;
        } else if (op == LC) {
            ax = *(char*)ax;
        } else if (op == LI) {
            ax = *(int*)ax;
        } else if (op == SC) {
            // ax = *(char*)*sp++ = ax;
            // the above code is from origin article
            *(char*)*sp++ = ax;
        } else if (op == SI) {
            *(int*)*sp++ = ax;
        } else if (op == PUSH) {
            *--sp = ax;
        } else if (op == JMP) {
            pc = (int*)*pc;
        } else if (op == JZ) {
            pc = ax ? pc + 1 : (int*)*pc;
        } else if (op == JNZ) {
            pc = ax ? (int*)*pc : pc + 1;
        } else if (op == CALL) {
            *--sp = (int)(pc + 1);
            pc = (int*)*pc;
        } else if (op == ENT) {
            *--sp = (int)bp;
            bp = sp;
            sp = sp - *pc++;
        } else if (op == ADJ) {
            sp = sp + *pc++;
        } else if (op == LEV) {
            sp = bp;
            bp = (int*)*sp++;
            pc = (int*)*sp++;
        } else if (op == LEA) {
            ax = (int)(bp + *pc++);
        } else if (op == OR) {
            ax = *sp++ | ax;
        } else if (op == XOR) {
            ax = *sp++ ^ ax;
        } else if (op == AND) {
            ax = *sp++ & ax;
        } else if (op == EQ) {
            ax = *sp++ == ax;
        } else if (op == NE) {
            ax = *sp++ != ax;
        } else if (op == LT) {
            ax = *sp++ < ax;
        } else if (op == LE) {
            ax = *sp++ <= ax;
        } else if (op == GT) {
            ax = *sp++ > ax;
        } else if (op == GE) {
            ax = *sp++ >= ax;
        } else if (op == SHL) {
            ax = *sp++ << ax;
        } else if (op == SHR) {
            ax = *sp++ >> ax;
        } else if (op == ADD) {
            ax = *sp++ + ax;
        } else if (op == SUB) {
            ax = *sp++ - ax;
        } else if (op == MUL) {
            ax = *sp++ * ax;
        } else if (op == DIV) {
            ax = *sp++ / ax;
        } else if (op == MOD) {
            ax = *sp++ % ax;
        } else if (op == EXIT) {
            printf("exit(%d)", *sp);
            return *sp;
        } else if (op == OPEN) {
            ax = open((char*)sp[1], sp[0]);
        } else if (op == CLOS) {
            ax = close(*sp);
        } else if (op == READ) {
            ax = read(sp[2], (char*)sp[1], *sp);
        } else if (op == PRTF) {
            tmp = sp + pc[1];
            ax = printf((char*)tmp[-1], tmp[-2], tmp[-3], tmp[-4], tmp[-5], tmp[-6]);
        } else if (op == MALC) {
            ax = (int)malloc(*sp);
        } else if (op == MSET) {
            ax = (int)memset((char*)sp[2], sp[1], *sp);
        } else if (op == MCMP) {
            ax = memcmp((char*)sp[2], (char*)sp[1], *sp);
        } else {
            printf("unknown instruction:%d\n", op);
            return -1;
        }
    }
    // *sp++ is POP operation
    return 0;
}  

int main(int argc, char **argv) {
    int i, fd;
    
    argc--;
    argv++;

    poolsize = 256 * 1024; // arbitrary size
    line = 1;

    if ((fd = open(*argv, 0)) < 0) {
        printf("Could not open(%s)\n", *argv);
        return -1;
    }

    if ( ! (src = old_src = malloc(poolsize))) {
        printf("Could not malloc(%d) for source area\n", poolsize);
        return -1;
    }

    // read the source file
    if ((i = read(fd, src, poolsize - 1)) <= 0) {
        printf("read() returned %d\n", i);
        return -1;
    }
    src[i] = 0;
    close(fd);

    // allocate memory for virtual machine
    if ( ! (text = old_text = malloc(poolsize))) {
        printf("Could not malloc(%d) for text area \n", poolsize);
        return -1;
    }
    if ( ! (data = malloc(poolsize))) {
        printf("Could not malloc(%d) for data area \n", poolsize);
        return -1;
    }
    memset(text, 0, poolsize);
    memset(data, 0, poolsize);
    memset(stack, 0, poolsize);

    bp = sp = (int*)((int)stack + poolsize); 
    ax = 0;

    i = 0;
    text[i++] = IMM;
    text[i++] = 10;
    text[i++] = PUSH;
    text[i++] = IMM;
    text[i++] = 20;
    text[i++] = ADD;
    text[i++] = PUSH;
    text[i++] = EXIT;

    pc = text;

    program();
    return eval();
}