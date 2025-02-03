#include <string.h>
#include <errno.h>

#include <elog.h>

#include "ush.h"
#include "state.h"
#include "console.h"


#undef UAIO_ARG1
#undef UAIO_ARG2
#undef UAIO_ENTITY
#define UAIO_ENTITY ush
#include "uaio_generic.c"


struct ush *
ush_create(struct euart_device *terminal, struct ush_command commands[]) {
    struct ush *sh = NULL;

    if (terminal == NULL) {
        return NULL;
    }

    sh = malloc(sizeof(struct ush));
    if (sh == NULL) {
        return NULL;
    }

    if (console_init(&sh->console, terminal)) {
        goto failed;
    }

    sh->commands = commands;
    return sh;

failed:
    console_deinit(&sh->console);

    if (sh) {
        free(sh);
    }

    return NULL;
}


int
ush_destroy(struct ush *sh) {
    int ret = 0;

    if (sh == NULL) {
        return -1;
    }

    ret |= console_deinit(&sh->console);

    free(sh);
    return ret;
}


ASYNC
ushA(struct uaio_task *self, struct ush *sh) {
    struct console *con = &sh->console;
    UAIO_BEGIN(self);

    /* loop */
    console_printf(con, LINEBREAK);
    while (true) {
        CONSOLE_PROMPT(con);
        fflush(stdout);
        CONSOLE_AREADLINE(self, con);
        if (UAIO_HASERROR(self)) {
            ERROR("console read error");
            console_printf(con, LINEBREAK);
            continue;
        }
        DEBUG("command: %.*s", con->linesize, con->line);
    }

    /* termination */
    UAIO_FINALLY(self);
}
