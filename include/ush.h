#ifndef USH_H_
#define USH_H_


#include <euart.h>
#include <uaio.h>


typedef struct ush_process {
    int argc;
    char **argv;
    void *userptr;
} ush_process_t;


#undef UAIO_ARG1
#undef UAIO_ARG2
#undef UAIO_ENTITY
#define UAIO_ENTITY ush_process
#include "uaio_generic.h"


struct ush_command {
    const char *name;
    ush_process_coro_t entrypoint;
};


struct ush_history {
    int head;
    int tail;
    char *commands[CONFIG_USH_HISTORY_MASK + 1];
};


typedef struct ush {
    /* console */
    struct euart console;

    /* typing */
#ifdef CONFIG_USH_VIMODE
    bool insertmode;
#endif
    char currentchar;
    char cmdline[CONFIG_USH_CMDLINE_MAX + 1];
    unsigned int cmdsize;
    unsigned int cursor;

    /* history */
    int historyoffset;
    struct ush_history history;

    /* user provided command vector */
    struct ush_command commands[];
} ush_t;


#undef UAIO_ARG1
#undef UAIO_ARG2
#undef UAIO_ENTITY
#define UAIO_ENTITY ush
#include "uaio_generic.h"


ASYNC
ushA(struct uaio_task *self, struct ush *sh);


#endif  // USH_H_
