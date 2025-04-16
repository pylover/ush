#ifndef USH__H_
#define USH__H_


#include "cmd.h"
#include "ush.h"
#include "term.h"


typedef struct ush {
    struct term term;
    struct ush_process *executing;

    /* builtin commands vector */
    struct ush_executable *builtins;

    /* user provided commands vector */
    struct ush_executable *commands;
} ush_t;


struct ush_executable*
ush_exec_find(struct ush *sh, const char *name);


#endif  // USH__H_
