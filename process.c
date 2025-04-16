#include <string.h>
#include <elog.h>
#include <uaio.h>

#include "ush.h"
#include "config.h"
#include "ush_.h"
#include "term.h"
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
        argv = realloc(argv, (argc + 1) * sizeof(char*));
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


struct ush_process *
process_create(struct ush *sh, struct cmd *cmd) {
    char *b;
    struct ush_process *p;
    struct ush_executable *exe;

    p = malloc(sizeof(struct ush_process));
    if (p == NULL) {
        return NULL;
    }

    b = malloc(cmd->len + 1);
    if (b == NULL) {
        free(p);
        return NULL;
    }
    memcpy(b, cmd->buff, cmd->len);
    b[cmd->len] = 0;
    p->buff = b;

    if (_tokenize(p)) {
        free(b);
        free(p);
        return NULL;
    }

    /* find entrypoint */
    if (p->argv == NULL) {
        free(b);
        free(p);
        return NULL;
    }

    exe = ush_exec_find(sh, p->argv[0]);
    if (exe == NULL) {
        term_printf(&sh->term, "Command '%s' not found%s", p->argv[0],
                LINEBREAK);
        free(p->argv);
        free(b);
        free(p);
        return NULL;
    }

    return p;
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

    free(p);
}
