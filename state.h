#ifndef STATE_H_
#define STATE_H_


#include "ush.h"
#include "console.h"


typedef struct ush {
    struct console console;

    /* user provided commands vector */
    struct ush_command *commands;
} ush_t;


#endif
