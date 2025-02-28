#ifndef STATE_H_
#define STATE_H_


#include "ush.h"
#include "term.h"


typedef struct ush {
    struct term term;
    struct str executing;

    /* user provided commands vector */
    struct ush_executable *commands;
} ush_t;


#endif
