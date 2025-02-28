#ifndef TERM_H_
#define TERM_H_


#include <euart.h>

#include "ansi.h"
#include "cmdring.h"


typedef struct term {
    struct euart_device *device;
    struct euart_reader reader;
    struct cmdring history;
} term_t;


#undef UAIO_ARG1
#undef UAIO_ARG2
#undef UAIO_ENTITY
#define UAIO_ENTITY term
#include "uaio_generic.h"


#define TERM_OUTFD(t) (t)->device->outfd
#define TERM_PROMPT(t) \
    term_printf(t, "%s%s:# ", ANSI_RESET, CONFIG_USH_PROMPT)
#define TERM_AWAIT(task, coro, t) \
    UAIO_AWAIT(task, term, coro, t)
#define TERM_AREADLINE(task, t) \
    TERM_AWAIT(task, term_readA, t)


int
term_init(struct term *term, struct euart_device *device);


int
term_deinit(struct term *term);


int
term_printf(struct term *term, const char *restrict fmt, ...);


int
term_append(struct term *term, char c);


ASYNC
term_readA(struct uaio_task *self, struct term *term);


#endif
