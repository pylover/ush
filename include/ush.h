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


typedef struct ush_process ush_process_t;
#undef UAIO_ARG1
#undef UAIO_ARG2
#undef UAIO_ENTITY
#define UAIO_ENTITY ush_process
#include <uaio_generic.h>


struct ush_process {
    char *buff;
    int argc;
    char **argv;
    ush_process_coro_t entrypoint;
};


struct ush_executable {
    const char *name;
    ush_process_coro_t entrypoint;
};


struct ush *
ush_create(struct euart_device *console, struct ush_executable commands[]);


int
ush_destroy(struct ush *sh);


ASYNC
ushA(struct uaio_task *self, struct ush *sh);


#endif  // USH_H_
