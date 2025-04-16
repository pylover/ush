#include <string.h>
#include <elog.h>
#include <uaio.h>

#include "ush.h"
#include "cmd.h"
#include "process.h"


#undef UAIO_ARG1
#undef UAIO_ARG2
#undef UAIO_ENTITY
#define UAIO_ENTITY ush_process
#include <uaio_generic.c>


static int
_tokenize(struct ush_process *p) {
    char *s;
    char *tmp = NULL;
    char *tok;
    char **argv = NULL;
    int argc = 0;

    for (s = p->buff; ; s = NULL, argc++) {
        tok = strtok_r(s, " ", &tmp);
        if (tok == NULL) {
            break;
        }
        errno = 0;
        argv = realloc(argv, argc + 1);
        if (errno == ENOMEM) {
            if (argv) {
                free(argv);
            }
            break;
        }
        argv[argc] = tok;
    }

    p->argc = argc;
    p->argv = argv;
    return 0;
}


int
process_fromcmd(struct ush_process *p, struct cmd *cmd) {
    char *b;

    if (p->buff) {
        return -1;
    }

    b = malloc(cmd->len + 1);
    if (b == NULL) {
        return -1;
    }
    memcpy(b, cmd->buff, cmd->len);
    b[cmd->len] = 0;
    p->buff = b;

    if (_tokenize(p)) {
        free(b);
        p->buff = NULL;
        return -1;
    }

    return 0;
}


void
process_free(struct ush_process *p) {
    if (p->buff) {
        free(p->buff);
        p->buff = NULL;
    }

    if (p->argv) {
        free(p->argv);
        p->argv = NULL;
        p->argc = 0;
    }
}
