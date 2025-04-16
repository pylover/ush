#include <string.h>
#include <errno.h>

#include <elog.h>

#include "config.h"
#include "builtins.h"
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
    sh->subprocess = NULL;;
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

    if (sh->subprocess) {
        process_free(sh->subprocess);
    }
    ret |= term_deinit(&sh->term);

    free(sh);
    return ret;
}


static const struct ush_executable*
_exec_find(struct ush *sh, const char *name) {
    /* look at builtins */
    if (!strcmp(name, builtin_free.name)) {
        return &builtin_free;
    }
    return NULL;
}


static ASYNC
_subprocessA(struct uaio_task *self, struct ush *sh) {
    const struct ush_executable *maincoro;
    struct ush_process *p;
    struct cmd *cmd;
    // ush_process_coro_t mainfunc;
    UAIO_BEGIN(self);

    /* get the terminal command line */
    cmd = TERM_CMDLINE(&sh->term);

    /* create and allocate a process */
    p = process_create(sh, cmd->buff, cmd->len);
    if (p == NULL) {
        UAIO_THROW(self);
    }
    DEBUG("new process: %s", p->buff);

    /* find entrypoint (main function) */
    maincoro = _exec_find(sh, p->argv[0]);
    if (maincoro == NULL) {
        term_printf(&sh->term, "Command '%s' not found%s", p->argv[0],
                LINEBREAK);
        process_free(p);
        UAIO_THROW(self);
    }
    sh->subprocess = p;

    // TODO: execute the process
    for (int i = 0; i < p->argc; i++) {
        DEBUG("[%d] %s", i, p->argv[i]);
    }
    PROCESS_AWAIT(self, maincoro->main, p);

    UAIO_FINALLY(self);
    if (sh->subprocess) {
        process_free(sh->subprocess);
        sh->subprocess = NULL;
    }
}


ASYNC
ushA(struct uaio_task *self, struct ush *sh) {
    UAIO_BEGIN(self);

    /* loop */
    while (true) {
        TERM_AREADLINE(self, &sh->term);
        if (UAIO_HASERROR(self)) {
            ERROR("term read error");
            continue;
        }

        USH_AWAIT(self, _subprocessA, sh);
    }

    /* termination */
    UAIO_FINALLY(self);
}
