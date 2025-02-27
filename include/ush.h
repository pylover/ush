#ifndef USH_H_
#define USH_H_


#include <euart.h>
#include <uaio.h>


#ifdef CONFIG_USH_LINEBREAK_LF
#define LINEBREAK "\n"
#elif CONFIG_USH_LINEBREAK_CRLF
#define LINEBREAK "\r\n"
#endif


typedef struct ush ush_t;
#undef UAIO_ARG1
#undef UAIO_ARG2
#undef UAIO_ENTITY
#define UAIO_ENTITY ush
#include "uaio_generic.h"


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


// TODO: rename to ush_executable
struct ush_command {
    const char *name;
    ush_process_coro_t entrypoint;
};


struct ush *
ush_create(struct euart_device *terminal, struct ush_command commands[]);


int
ush_destroy(struct ush *sh);


ASYNC
ushA(struct uaio_task *self, struct ush *sh);


#endif  // USH_H_
