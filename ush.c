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
    sh->executing = NULL;
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

    process_free(sh->executing);
    sh->executing = NULL;
    ret |= term_deinit(&sh->term);

    free(sh);
    return ret;
}


struct ush_executable*
ush_exec_find(struct ush *sh, const char *name) {
    return NULL;
}


ASYNC
ushA(struct uaio_task *self, struct ush *sh) {
    struct term *term = &sh->term;
    struct ush_process *p;
    UAIO_BEGIN(self);

    /* loop */
    while (true) {
        TERM_AREADLINE(self, term);
        if (UAIO_HASERROR(self)) {
            ERROR("term read error");
            continue;
        }
        p = process_create(sh, TERM_CMDLINE(term));
        if (p == NULL) {
            continue;
        }
        DEBUG("new process: %s", p->buff);
        sh->executing = p;
        // TODO: execute the process
        for (int i = 0; i < p->argc; i++) {
            DEBUG("[%d] %s", i, p->argv[i]);
        }
        process_free(sh->executing);
    }

    /* termination */
    UAIO_FINALLY(self);
}
