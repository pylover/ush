#ifndef STATE_H_
#define STATE_H_


#include "ush.h"
#include "terminal.h"


typedef struct ush {
    struct terminal terminal;

    /* user provided commands vector */
    struct ush_executable *commands;
} ush_t;


#endif
