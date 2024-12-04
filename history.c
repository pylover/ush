#include <errno.h>

#include <elog.h>

#include "history.h"


/*
 * 01234567
 *
 * abcdefg
 * t      h
 *
 * foo
 * t  h
 *
 * oo     f
 *   h    t
 *
 *
 * h == t -> empty
 * used = (h - t) & MASK
 * free = (t - h) & MASK - 1
 */


#define HISTORY_MASK(x) ((x) & CONFIG_USH_HISTORY_MASK)
#define HISTORY_USED(h) HISTORY_MASK((h)->head - (h)->tail)
#define HISTORY_FREE(h) (HISTORY_MASK((h)->tail - (h)->head) - 1)
#define HISTORY_INCR(x) HISTORY_MASK((x) + 1)
#define HISTORY_HEADOFF(sh) \
    HISTORY_MASK((sh)->history.head + (sh)->historyoffset)
#define HISTORY_CURRENT(sh) (sh)->history.commands[HISTORY_HEADOFF(sh)]


void
history_init(struct ush *sh) {
    memset(&sh->history, 0, sizeof(sh->history));
    sh->historyoffset = 0;
}


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


int
history_next(struct ush *sh) {
    if (!sh->historyoffset) {
        return 0;
    }

    sh->historyoffset++;
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
