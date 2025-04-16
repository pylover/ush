#include <string.h>
#include <errno.h>

#include <elog.h>

#include "config.h"
#include "ush.h"
#include "process.h"
#include "ush_.h"
#include "term.h"


#undef UAIO_ARG1
#undef UAIO_ARG2
#undef UAIO_ENTITY
#define UAIO_ENTITY ush
#include "uaio_generic.c"


struct ush *
ush_create(struct euart_device *console, struct ush_executable commands[]) {
    struct ush *sh = NULL;

    if (console == NULL) {
        return NULL;
    }

    sh = malloc(sizeof(struct ush));
    if (sh == NULL) {
        return NULL;
    }

    if (term_init(&sh->term, console->infd, console->outfd)) {
        goto failed;
    }

    sh->commands = commands;
    sh->executing.buff = NULL;
    sh->executing.argv = NULL;
    sh->executing.argc = 0;
    return sh;

failed:
    term_deinit(&sh->term);
    free(sh);
    return NULL;
}


int
ush_destroy(struct ush *sh) {
    int ret = 0;

    if (sh == NULL) {
        return -1;
    }

    process_free(&sh->executing);
    ret |= term_deinit(&sh->term);

    free(sh);
    return ret;
}


ASYNC
ushA(struct uaio_task *self, struct ush *sh) {
    struct term *term = &sh->term;
    struct ush_process *p = &sh->executing;
    UAIO_BEGIN(self);

    /* loop */
    while (true) {
        process_free(p);
        TERM_AREADLINE(self, term);
        if (UAIO_HASERROR(self)) {
            ERROR("term read error");
            continue;
        }
        if (process_fromcmd(p, TERM_CMDLINE(term))) {
            ERROR("process create");
            continue;
        }
        DEBUG("new process: %s", p->buff);
        for (int i = 0; i < p->argc; i++) {
            DEBUG("[%d] %s", i, p->argv[i]);
        }
    }

    /* termination */
    UAIO_FINALLY(self);
}
