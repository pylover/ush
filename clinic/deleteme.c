// static int
// _insert(struct ush *sh, char c) {
//     term_insert(sh, sh->currentchar);
//     sh->currentchar = c;
//     return 0;
// }


// static ASYNC
// _ansiA(struct uaio_task *self, struct ush *sh) {
//     UAIO_BEGIN(self);
//
//     /* read [ */
//     _read.timeout_us = 10000;
//     EUART_AWAIT(self, euart_getcA, &sh->console, &_read);
//     if ((_read.status != EUCS_OK) || (_read.c != '[')) {
//         _insert(sh, _read.c);
//         UAIO_RETURN(self);
//     }
//
//     // TODO: continue from here
//     // /* read ansi chars */
//     // char ctrl[4];
//     // _chat.query.count = 3;
//     // _chat.result.str = ctrl;
//     // UAIO_AWAIT(self, euart, euart_readA, &sh->console, &_chat);
//     // if (_chat.status != EUCS_OK) {
//     //     ERROR("Invalid ANSI control chars: %s");
//     // }
//
//     // DEBUG("strlen: %d", strlen(ctrl));
//     // if (strlen(ctrl) == 1) {
//     //     DEBUG("Ansi codes: %d", ctrl[0]);
//     // }
//     // else if (strlen(ctrl) == 2) {
//     //     DEBUG("Ansi codes: %d %d", ctrl[0], ctrl[1]);
//     // }
//     UAIO_FINALLY(self);
// //     int ret;
// //     if (need > NEED_ANY) {
// //         // ret = read(sh->console.infd, &c, 1);
// //         // DEBUG("need: %d %c", ret, c);
// //         // if (ret == -1) {
// //         //     UAIO_AWAIT(self, euart, euart_getcA, &sh->console, &_chat);
// //         // }
// //
// // need:
// //         UAIO_AWAIT(self, euart, euart_getcA, &sh->console, &_chat);
// //         if (_chat.status == EUCS_TIMEDOUT) {
// //             DEBUG("need: read timedout");
// //             goto need;
// //         }
// //         if (UAIO_FILE_TIMEDOUT(self)) {
// //         }
// //         c = _chat.result.uint8;
// //
// //         if (need == c) {
// //             DEBUG("need: %c", need);
// //             need = NEED_ANY;
// //         }
// //         else {
// //             goto failed;
// //         }
// //     }
// //
// //     /* read any char */
// //     UAIO_AWAIT(self, euart, euart_getcA, &sh->console, &_chat);
// //     c = _chat.result.uint8;
// //     DEBUG("any: %c", c);
// //
// //     /*
// //     DEL
// //     27
// //     91 [
// //     51 3
// //     126 ~
// //     */
// //     switch (c) {
// //         /* Delete */
// //         case '3':
// //         case '2':
// //         case '1':
// //         case '4':
// //             DEBUG("case 3");
// //             ret = read(sh->console.infd, &c, 1);
// //             DEBUG("case 30: %d %c", ret, c);
// //
// // // case3:
// //             // UAIO_AWAIT(self, euart, euart_getcA, &sh->console, &_chat);
// //             // if (UAIO_FILE_TIMEDOUT(self)) {
// //             //     DEBUG("need: read timedout");
// //             //     goto case3;
// //             // }
// //             // if (!_chat.status == EUCS_OK) {
// //             //     DEBUG("case 3 read error: %d", errno);
// //             //     goto case3;
// //             // }
// //             // c = _chat.result.uint8;
// //             // DEBUG("%c", c);
// //
// //             if (c == '~') {
// //                 term_delete(sh);
// //             }
// //             break;
// //
// //         /* Left */
// //         case 'D':
// //             term_navleft(sh, 1);
// //             break;
// //
// //         /* Right */
// //         case 'C':
// //             term_navright(sh, 1);
// //             break;
// //
// //         /* Up */
// //         case 'A':
// //             history_updatecurrent(sh);
// //             if (history_prev(sh)) {
// //                 term_rewrite(sh);
// //             }
// //             break;
// //
// //         /* Buttom */
// //         case 'B':
// //             history_updatecurrent(sh);
// //             if (history_next(sh)) {
// //                 term_rewrite(sh);
// //             }
// //             break;
// //     }
// //
// // failed:
// // done:
// }


// static ASYNC
// _navA(struct uaio_task *self, struct ush *sh) {
//     UAIO_BEGIN(self);
//
//     if (sh->currentchar == CHAR_BACKSPACE) {
//         term_backspace(sh);
//     }
//     else if (sh->currentchar == CHAR_ESCAPE) {
//         UAIO_AWAIT(self, ush, _ansiA, sh);
//     }
//     else {
//         UAIO_THROW(self, EINVAL);
//     }
//
//     UAIO_FINALLY(self);
// }
//
//
// static ASYNC
// _execA(struct uaio_task *self, struct ush *sh) {
//     // TODO: tokenize
//     // TODO: find command
//     UAIO_BEGIN(self);
//     DEBUG("executing: %s", sh->cmdline);
//
//     if (sh->cmdsize) {
//         printf("%.*s\n", sh->cmdsize, sh->cmdline);
//     }
//     UAIO_FINALLY(self);
// }



void
history_alloc(struct ush *sh) {
    struct ush_history *h = &sh->history;

    if (!HISTORY_FREE(h)) {
        /* tail drop */
        h->tail = HISTORY_INCR(h->tail);
    }

    h->head = HISTORY_INCR(h->head);
    sh->historyoffset = 0;
    char *cur = HISTORY_CURRENT(sh);
    if (cur) {
        free(cur);
        HISTORY_CURRENT(sh) = NULL;
    }
}


void
history_updatecurrent(struct ush *sh) {
    if (sh->cmdsize == 0) {
        return;
    }
    int newsize = sh->cmdsize + 1;
    char *cur = HISTORY_CURRENT(sh);

    if (cur == NULL) {
        cur = malloc(newsize);
    }
    else if (strlen(cur) != newsize) {
        cur = realloc(cur, newsize);
    }

    cur[newsize - 1] = '\0';
    HISTORY_CURRENT(sh) = cur;
    strncpy(cur, sh->cmdline, sh->cmdsize);
}


int
history_prev(struct ush *sh) {
    struct ush_history *h = &sh->history;

    if (HISTORY_USED(h) <= abs(sh->historyoffset)) {
        return 0;
    }

    sh->historyoffset--;
    char *cur = HISTORY_CURRENT(sh);
    if (cur) {
        strcpy(sh->cmdline, cur);
        sh->cmdsize = strlen(cur);
    }
    else {
        sh->cmdsize = 0;
    }
    return 1;
}


char *
history_next(struct ush_history *h) {
    if (!h->offset) {
        return NULL;
    }

    h->offset++;
    return HISTORY_CURRENT(h);
}


//         // /* debug */
//         // DEBUG("%d %c", sh->currentchar, sh->currentchar);
//         // continue;
//
//         /* ansi and arrow keys */
//         if (ISNAV(sh->currentchar)) {
//             UAIO_AWAIT(self, ush, _navA, sh);
//             continue;
//         }
//
//         /* enter */
//         if (sh->currentchar == CHAR_LF) {
//             term_navend(sh);
//             putchar('\n');
//             sh->historyoffset = 0;
//             if (sh->cmdsize) {
//                 sh->cmdline[sh->cmdsize] = '\0';
//                 history_updatecurrent(sh);
//                 UAIO_AWAIT(self, ush, _execA, sh);
//                 sh->cmdsize = 0;
//                 history_alloc(sh);
//             }
//             sh->cursor = 0;
//             goto prompt;
//         }
//
//         /* insert char */
//         if (term_insert(sh, sh->currentchar)) {
//             UAIO_THROW(self, ENOMEM);
//         }


