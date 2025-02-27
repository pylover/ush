#ifndef TERMINAL_H_
#define TERMINAL_H_


#include <euart.h>

#include "ansi.h"
#include "history.h"


typedef struct terminal {
    struct euart_reader reader;
    struct history history;

    unsigned int linesize;
    char line[CONFIG_USH_TERMINAL_LINESIZE];
} terminal_t;


#undef UAIO_ARG1
#undef UAIO_ARG2
#undef UAIO_ENTITY
#define UAIO_ENTITY terminal
#include "uaio_generic.h"


#define TERMINAL_PROMPT(con) \
    terminal_printf(con, "%s%s:# ", ANSI_RESET, CONFIG_USH_PROMPT)
#define TERMINAL_AWAIT(task, coro, con) UAIO_AWAIT(task, terminal, coro, con)
#define TERMINAL_AREADLINE(task, con) TERMINAL_AWAIT(task, terminal_readA, con)


int
terminal_init(struct terminal *c, struct euart_device *device);


int
terminal_deinit(struct terminal *c);


int
terminal_printf(struct terminal *c, const char *restrict fmt, ...);


ASYNC
terminal_readA(struct uaio_task *self, struct terminal *c);


#endif
