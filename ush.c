#include <string.h>
#include <errno.h>

#include <elog.h>

#include "config.h"
#include "ush.h"
#include "state.h"
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

    if (cmd_init(&sh->executing, CONFIG_USH_TERM_LINESIZE)) {
        goto failed;
    }

    sh->commands = commands;
    return sh;

failed:
    term_deinit(&sh->term);
    cmd_deinit(&sh->executing);
    free(sh);
    return NULL;
}


int
ush_destroy(struct ush *sh) {
    int ret = 0;

    if (sh == NULL) {
        return -1;
    }

    cmd_deinit(&sh->executing);
    ret |= term_deinit(&sh->term);

    free(sh);
    return ret;
}


ASYNC
ushA(struct uaio_task *self, struct ush *sh) {
    struct term *term = &sh->term;
    struct cmd *cmd = &sh->executing;
    UAIO_BEGIN(self);

    /* loop */
    while (true) {
        cmd_clear(cmd);
        TERM_AREADLINE(self, term);
        if (UAIO_HASERROR(self)) {
            ERROR("term read error");
            continue;
        }
        cmd_copy(cmd, TERM_CMDLINE(term));
        DEBUG("command: %.*s", cmd->len, cmd->buff);
    }

    /* termination */
    UAIO_FINALLY(self);
}
