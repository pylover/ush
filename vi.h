#ifndef VI_H_
#define VI_H_


#include <uaio.h>


enum vi_mode {
    VI_NORMAL,
    VI_INSERT,
};


struct term;
typedef struct vi {
    enum vi_mode mode;
    int repeat;
    struct term *term;
} vi_t;


#undef UAIO_ARG1
#undef UAIO_ARG2
#undef UAIO_ENTITY
#define UAIO_ENTITY vi
#include "uaio_generic.h"


#define VI_AWAIT(task, coro, v) UAIO_AWAIT(task, vi, coro, v)
#define vi_mode(vi) (vi)->mode


void
vi_init(struct vi *vi, struct term *term);


void
vi_swmode(struct vi *vi, enum vi_mode mode);


ASYNC
viA(struct uaio_task *self, struct vi *vi);


#endif  // VI_H_
