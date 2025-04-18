#ifndef USH_H_
#define USH_H_


#include <euart.h>
#include <uaio.h>


typedef struct ush ush_t;
#undef UAIO_ARG1
#undef UAIO_ARG2
#undef UAIO_ENTITY
#define UAIO_ENTITY ush
#include <uaio_generic.h>
#define USH_AWAIT(task, coro, t) UAIO_AWAIT(task, ush, coro, t)


typedef struct ush_process ush_process_t;
#undef UAIO_ARG1
#undef UAIO_ARG2
#undef UAIO_ENTITY
#define UAIO_ENTITY ush_process
#include <uaio_generic.h>
#define PROCESS_AWAIT(task, coro, t) UAIO_AWAIT(task, ush_process, coro, t)


struct ush_process {
    struct term *term;
    char *buff;
    void *userptr;
    int argc;
    char **argv;
};


struct ush_executable {
    const char *name;
    ush_process_coro_t main;
};


struct ush *
ush_create(struct euart_device *console, struct ush_executable commands[]);


int
ush_destroy(struct ush *sh);


int
ush_printf(struct ush_process *p, const char *restrict fmt, ...);


ASYNC
ushA(struct uaio_task *self, struct ush *sh);


#endif  // USH_H_
