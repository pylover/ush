#ifndef USH__H_
#define USH__H_


#include "cmd.h"
#include "ush.h"
#include "term.h"


typedef struct ush {
    struct term term;
    struct ush_process executing;

    /* user provided commands vector */
    struct ush_executable *commands;
} ush_t;


#endif  // USH__H_
