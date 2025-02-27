#include <string.h>
#include <errno.h>

#include <elog.h>

#include "config.h"
#include "ush.h"
#include "state.h"
#include "terminal.h"


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

    if (terminal_init(&sh->terminal, console)) {
        goto failed;
    }

    sh->commands = commands;
    return sh;

failed:
    terminal_deinit(&sh->terminal);

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

    ret |= terminal_deinit(&sh->terminal);

    free(sh);
    return ret;
}


ASYNC
ushA(struct uaio_task *self, struct ush *sh) {
    struct terminal *con = &sh->terminal;
    UAIO_BEGIN(self);

    /* loop */
    terminal_printf(con, LINEBREAK);
    while (true) {
        TERMINAL_PROMPT(con);
        fflush(stdout);
        TERMINAL_AREADLINE(self, con);
        if (UAIO_HASERROR(self)) {
            ERROR("terminal read error");
            terminal_printf(con, LINEBREAK);
            continue;
        }
        DEBUG("command: %.*s", con->linesize, con->line);
    }

    /* termination */
    UAIO_FINALLY(self);
}
