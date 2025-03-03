#ifndef TERM_H_
#define TERM_H_


#include <euart.h>

#include "ansi.h"
#include "cmd.h"


#undef ERING_PREFIX
#define ERING_PREFIX cmd
#include <ering.h>


typedef struct term {
    int outfd;
    struct euart_reader reader;
    struct cmdring cmdring;
    unsigned int row;
    unsigned int col;
} term_t;


#undef UAIO_ARG1
#undef UAIO_ARG2
#undef UAIO_ENTITY
#define UAIO_ENTITY term
#define UAIO_ARG1 struct cmd*
#include "uaio_generic.h"


#define TERM_OUTFD(t) (t)->outfd
#define TERM_AWAIT(task, coro, t, o) \
    UAIO_AWAIT(task, term, coro, t, o)
#define TERM_AREADLINE(task, t, o) \
    TERM_AWAIT(task, term_readA, t, o)


int
term_init(struct term *term, int infd, int outfd);


int
term_deinit(struct term *term);


ASYNC
term_readA(struct uaio_task *self, struct term *term, struct cmd *out);


#endif
