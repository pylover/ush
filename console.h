#ifndef CONSOLE_H_
#define CONSOLE_H_


#include <euart.h>

#include "ansi.h"
#include "history.h"


typedef struct console {
    struct euart_reader reader;
    struct history history;

    unsigned int linesize;
    char line[CONFIG_USH_CONSOLE_LINESIZE];
} console_t;


#undef UAIO_ARG1
#undef UAIO_ARG2
#undef UAIO_ENTITY
#define UAIO_ENTITY console
#include "uaio_generic.h"


#define CONSOLE_PROMPT(con) \
    console_printf(con, "%s%s:# ", ANSI_RESET, CONFIG_USH_PROMPT)
#define CONSOLE_AWAIT(task, coro, con) UAIO_AWAIT(task, console, coro, con)
#define CONSOLE_AREADLINE(task, con) CONSOLE_AWAIT(task, console_readA, con)


int
console_init(struct console *c, struct euart_device *device);


int
console_deinit(struct console *c);


int
console_printf(struct console *c, const char *restrict fmt, ...);


ASYNC
console_readA(struct uaio_task *self, struct console *c);


#endif
