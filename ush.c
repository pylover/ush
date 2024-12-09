#include <string.h>
#include <errno.h>

#include <elog.h>

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


static struct euart_read _read;


static ASYNC
_ansiA(struct uaio_task *self, struct ush *sh) {
    UAIO_BEGIN(self);
    DEBUG("ansiA");

    /* read [ */
    _read.timeout_us = 10000;
    EUART_AWAIT(self, euart_getcA, &sh->console, &_read);
    if ((_read.status != EUCS_OK) || (_read.c != '[')) {
        term_insert(sh, sh->currentchar);
        UAIO_RETURN(self);
    }

    // TODO: continue from here
    // /* read ansi chars */
    // char ctrl[4];
    // _chat.query.count = 3;
    // _chat.result.str = ctrl;
    // UAIO_AWAIT(self, euart, euart_readA, &sh->console, &_chat);
    // if (_chat.status != EUCS_OK) {
    //     ERROR("Invalid ANSI control chars: %s");
    // }

    // DEBUG("strlen: %d", strlen(ctrl));
    // if (strlen(ctrl) == 1) {
    //     DEBUG("Ansi codes: %d", ctrl[0]);
    // }
    // else if (strlen(ctrl) == 2) {
    //     DEBUG("Ansi codes: %d %d", ctrl[0], ctrl[1]);
    // }
    UAIO_FINALLY(self);
//     int ret;
//     if (need > NEED_ANY) {
//         // ret = read(sh->console.infd, &c, 1);
//         // DEBUG("need: %d %c", ret, c);
//         // if (ret == -1) {
//         //     UAIO_AWAIT(self, euart, euart_getcA, &sh->console, &_chat);
//         // }
//
// need:
//         UAIO_AWAIT(self, euart, euart_getcA, &sh->console, &_chat);
//         if (_chat.status == EUCS_TIMEDOUT) {
//             DEBUG("need: read timedout");
//             goto need;
//         }
//         if (UAIO_FILE_TIMEDOUT(self)) {
//         }
//         c = _chat.result.uint8;
//
//         if (need == c) {
//             DEBUG("need: %c", need);
//             need = NEED_ANY;
//         }
//         else {
//             goto failed;
//         }
//     }
//
//     /* read any char */
//     UAIO_AWAIT(self, euart, euart_getcA, &sh->console, &_chat);
//     c = _chat.result.uint8;
//     DEBUG("any: %c", c);
//
//     /*
//     DEL
//     27
//     91 [
//     51 3
//     126 ~
//     */
//     switch (c) {
//         /* Delete */
//         case '3':
//         case '2':
//         case '1':
//         case '4':
//             DEBUG("case 3");
//             ret = read(sh->console.infd, &c, 1);
//             DEBUG("case 30: %d %c", ret, c);
//
// // case3:
//             // UAIO_AWAIT(self, euart, euart_getcA, &sh->console, &_chat);
//             // if (UAIO_FILE_TIMEDOUT(self)) {
//             //     DEBUG("need: read timedout");
//             //     goto case3;
//             // }
//             // if (!_chat.status == EUCS_OK) {
//             //     DEBUG("case 3 read error: %d", errno);
//             //     goto case3;
//             // }
//             // c = _chat.result.uint8;
//             // DEBUG("%c", c);
//
//             if (c == '~') {
//                 term_delete(sh);
//             }
//             break;
//
//         /* Left */
//         case 'D':
//             term_navleft(sh, 1);
//             break;
//
//         /* Right */
//         case 'C':
//             term_navright(sh, 1);
//             break;
//
//         /* Up */
//         case 'A':
//             history_updatecurrent(sh);
//             if (history_prev(sh)) {
//                 term_rewrite(sh);
//             }
//             break;
//
//         /* Buttom */
//         case 'B':
//             history_updatecurrent(sh);
//             if (history_next(sh)) {
//                 term_rewrite(sh);
//             }
//             break;
//     }
//
// failed:
// done:
}


static ASYNC
_navA(struct uaio_task *self, struct ush *sh) {
    UAIO_BEGIN(self);

    if (sh->currentchar == CHAR_BACKSPACE) {
        term_backspace(sh);
    }
    else if (sh->currentchar == CHAR_ESCAPE) {
        UAIO_AWAIT(self, ush, _ansiA, sh);
    }
    else {
        UAIO_THROW(self, EINVAL);
    }

    UAIO_FINALLY(self);
}


static ASYNC
_execA(struct uaio_task *self, struct ush *sh) {
    // TODO: tokenize
    // TODO: find command
    UAIO_BEGIN(self);
    DEBUG("executing: %s", sh->cmdline);

    if (sh->cmdsize) {
        printf("%.*s\n", sh->cmdsize, sh->cmdline);
    }
    UAIO_FINALLY(self);
}


ASYNC
ushA(struct uaio_task *self, struct ush *sh) {
    // int ret;
    UAIO_BEGIN(self);

    sh->currentchar = 0;
    sh->cmdsize = 0;
    sh->cursor = 0;
    history_init(sh);

    memset(&_read, 0, sizeof(_read));
    // _read.timeout_us = 1000000;
    _read.timeout_us = 0;
    _read.max = CONFIG_USH_CMDLINE_MAX;
    _read.buff = malloc(CONFIG_USH_CMDLINE_MAX + 1);
    if (_read.buff == NULL) {
        UAIO_THROW(self, ENOMEM);
    }

prompt:
    printf("\033[0m");
    printf("root@esp:~# ");
    while (true) {
        fflush(stdout);
        EUART_AWAIT(self, euart_getcA, &sh->console, &_read);
        if (_read.status != EUCS_OK) {
            continue;
        }

        sh->currentchar = _read.c;

        // /* debug */
        // DEBUG("%d %c", sh->currentchar, sh->currentchar);
        // continue;

        /* ansi and arrow keys */
        if (ISNAV(sh->currentchar)) {
            UAIO_AWAIT(self, ush, _navA, sh);
            continue;
        }

        /* enter */
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

        /* insert char */
        term_insert(sh, sh->currentchar);
    }
    UAIO_FINALLY(self);
}
