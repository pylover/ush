#include <string.h>
#include <errno.h>

#include "ush.h"
#include "char.h"
#include "term.h"
#include "history.h"


#undef UAIO_ARG1
#undef UAIO_ARG2
#undef UAIO_ENTITY
#define UAIO_ENTITY ush
#include "uaio_generic.c"


#define NEED_ANY 0
#define NEED_NOTHING -1


static struct euart_chat _chat;



static ASYNC
_navA(struct uaio_task *self, struct ush *sh) {
    static char c;
    static int need;
    UAIO_BEGIN(self);

    if (sh->currentchar == CHAR_BACKSPACE) {
        term_backspace(sh);
        goto done;
    }

    if (sh->currentchar == CHAR_ESCAPE) {
        USH_DEBUG(sh, "escape");
#ifdef CONFIG_USH_VIMODE
        if (sh->insertmode) {
            USH_DEBUG(sh, "getting normal mode");
            sh->insertmode = false;
        }
#endif
        need = '[';
    }
    else {
        need = -1;
        c = sh->currentchar;
    }

    while (need != NEED_NOTHING) {
        UAIO_AWAIT(self, euart, euart_getc, &sh->console, &_chat);
        if (_chat.status != EUCS_OK) {
            continue;
        }

        c = _chat.result.uint8;
        if (need == NEED_ANY) {
            break;
        }

        if (need != c) {
            goto done;
        }

        need = NEED_ANY;
    }

    /*
    DEL
    27
    91 [
    51 3
    126 ~
    */
    switch (c) {
        // case '3':
        //     need = '~';

        case 'D':
            /* Left */
            term_navleft(sh, 1);
            break;

        case 'C':
            /* Right */
            term_navright(sh, 1);
            break;

        case 'A':
            /* Up */
            history_updatecurrent(sh);
            if (history_prev(sh)) {
                term_rewrite(sh);
            }
            break;

        case 'B':
            /* Buttom */
            history_updatecurrent(sh);
            if (history_next(sh)) {
                term_rewrite(sh);
            }
            break;
    }
done:
    fflush(stdout);
    UAIO_FINALLY(self);
}


static ASYNC
_execA(struct uaio_task *self, struct ush *sh) {
    // TODO: tokenize
    // TODO: find command
    UAIO_BEGIN(self);
    USH_DEBUG(sh, "executing: %s", sh->cmdline);

    if (sh->cmdsize) {
        printf("%.*s\n", sh->cmdsize, sh->cmdline);
    }
    UAIO_FINALLY(self);
}


ASYNC
ushA(struct uaio_task *self, struct ush *sh) {
    // int ret;
    UAIO_BEGIN(self);

#ifdef CONFIG_USH_VIMODE
    sh->insertmode = true;
#endif
    sh->currentchar = 0;
    sh->cmdsize = 0;
    sh->cursor = 0;
    history_init(sh);

    memset(&_chat, 0, sizeof(_chat));
    _chat.timeout_us = 1000000;

prompt:
    // printf("\033[0m");
    printf("\033[m");
    printf("root@esp:~# ");
    fflush(stdout);
    while (true) {
        UAIO_AWAIT(self, euart, euart_getc, &sh->console, &_chat);
        if (_chat.status != EUCS_OK) {
            continue;
        }

        sh->currentchar = _chat.result.uint8;

        // /* debug */
        // DEBUG("%d %c", sh->currentchar, sh->currentchar);
        // continue;

        /* ansi and arrow keys */
        if (ISNAV(sh->currentchar)) {
            UAIO_AWAIT(self, ush, _navA, sh);
            continue;
        }

        // if (sh->insertmode) {
        // }
        // else {
        //     /* normal mode */
        // }

        /* insert mode */
        if (sh->currentchar == CHAR_LF) {
            term_navend(sh);
            putchar('\n');
            sh->historyoffset = 0;
            if (sh->cmdsize) {
                sh->cmdline[sh->cmdsize] = '\0';
                history_updatecurrent(sh);
                UAIO_AWAIT(self, ush, _execA, sh);
                sh->cmdsize = 0;
                history_alloc(sh);
            }
            sh->cursor = 0;
            goto prompt;
        }
        term_insert(sh, sh->currentchar);
        fflush(stdout);
        // UAIO_SLEEP(self, 1000000);
    }
    UAIO_FINALLY(self);
}
