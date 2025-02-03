#include <errno.h>
#include <string.h>

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
#define HISTORY_HEADOFF(h) HISTORY_MASK((h)->head + (h)->offset)
#define HISTORY_CURRENT(h) (h)->commands[HISTORY_HEADOFF(h)]


struct ush_history *
history_create(unsigned int maskbits) {
    struct ush_history *h;
    unsigned int max;
    size_t size;

    if ((maskbits == 0) || (maskbits >= (sizeof(unsigned int) * 8))) {
        return NULL;
    }

    max = 1 << maskbits;
    size = sizeof(struct ush_history) + max * sizeof(char*);
    h = malloc(size);
    if (h == NULL) {
        return NULL;
    }

    memset(h, 0, size);
    h->max = max;
    h->mask = max - 1;

    return h;
}


int
history_dispose(struct ush_history *h) {
    if (h == NULL) {
        return -1;
    }

    free(h);
    return 0;
}
