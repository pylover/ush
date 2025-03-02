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

    if (term_init(&sh->term, console)) {
        goto failed;
    }

    if (str_init(&sh->executing, CONFIG_USH_TERM_LINESIZE)) {
        goto failed;
    }

    sh->commands = commands;
    return sh;

failed:
    term_deinit(&sh->term);
    str_deinit(&sh->executing);
    free(sh);
    return NULL;
}


int
ush_destroy(struct ush *sh) {
    int ret = 0;

    if (sh == NULL) {
        return -1;
    }

    str_deinit(&sh->executing);
    ret |= term_deinit(&sh->term);

    free(sh);
    return ret;
}


ASYNC
ushA(struct uaio_task *self, struct ush *sh) {
    struct term *term = &sh->term;
    struct str *cmd = &sh->executing;
    UAIO_BEGIN(self);

    /* loop */
    term_printf(term, LINEBREAK);
    while (true) {
        cmd->len = 0;
        TERM_AREADLINE(self, term, cmd);
        if (UAIO_HASERROR(self)) {
            ERROR("term read error");
            term_printf(term, LINEBREAK);
            continue;
        }
        if (cmd->len) {
            DEBUG("command: %.*s", cmd->len, cmd->start);
        }
    }

    /* termination */
    UAIO_FINALLY(self);
}
