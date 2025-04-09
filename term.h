#ifndef TERM_H_
#define TERM_H_


#include <euart.h>

#include "ansi.h"
#include "cmd.h"


#undef ERING_PREFIX
#define ERING_PREFIX cmd
#include <ering.h>


enum vi_mode {
    VI_NORMAL,
    VI_INSERT,
};


typedef struct term {
    int outfd;
    struct euart_reader reader;
    struct cmdring history;
    unsigned int rotation;
    unsigned int col;
#ifdef CONFIG_USH_VI
    enum vi_mode mode;
    int vi_repeat;
#endif  // CONFIG_USH_VI
} term_t;


#undef UAIO_ARG1
#undef UAIO_ARG2
#undef UAIO_ENTITY
#define UAIO_ENTITY term
#include "uaio_generic.h"


#define TERM_AWAIT(task, coro, t) UAIO_AWAIT(task, term, coro, t)
#define TERM_AREADLINE(task, t) TERM_AWAIT(task, term_readA, t)
#define TERM_INBUFF_COUNT(t) ERING_USED(&(t)->reader.ring)
#define TERM_INBUFF_POP(t) ERING_POP(&(t)->reader.ring)
#define TERM_INBUFF_GET(t) ERING_TAIL(&(t)->reader.ring)
#define TERM_INBUFF_GETOFF(t, off) ERING_TAILOFF(&(t)->reader.ring, off)
#define TERM_INBUFF_SKIP(t, n) ERING_SKIP(&(t)->reader.ring, n)
#define TERM_CMDLINE(t) ERING_HEADPTROFF(&(t)->history, (t)->rotation)
#define TERM_HISTORY_COUNT(t) ERING_USED(&(t)->history)
#define TERM_HISTORY_ISFULL(t) ERING_ISFULL(&(t)->history)
#define TERM_HISTORY_EXTEND(t) ERING_INCRHEAD(&(t)->history)
#define TERM_HISTORY_SHRINK(t) ERING_INCRTAIL(&(t)->history)
#define TERM_HISTORY_OFFSET(t, i) ERING_HEADPTROFF(&(t)->history, i)


int
term_init(struct term *term, int infd, int outfd);


int
term_deinit(struct term *term);


ASYNC
term_readA(struct uaio_task *self, struct term *term);


#endif
