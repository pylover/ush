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
#define UAIO_ARG1 struct str*
#include "uaio_generic.h"


#define TERM_OUTFD(t) (t)->device->outfd
#define TERM_AWAIT(task, coro, t, o) \
    UAIO_AWAIT(task, term, coro, t, o)
#define TERM_AREADLINE(task, t, o) \
    TERM_AWAIT(task, term_readA, t, o)


int
term_init(struct term *term, struct euart_device *device);


int
term_deinit(struct term *term);


int
term_printf(struct term *term, const char *restrict fmt, ...);


int
term_prompt(struct term *term);


int
term_append(struct term *term, char c);


ASYNC
term_readA(struct uaio_task *self, struct term *term, struct str *out);


#endif
