#ifndef STATE_H_
#define STATE_H_


#include "cmd.h"
#include "ush.h"
#include "term.h"


typedef struct ush {
    struct term term;
    struct cmd executing;

    /* user provided commands vector */
    struct ush_executable *commands;
} ush_t;


#endif
