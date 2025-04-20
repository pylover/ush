#include <stdlib.h>

#include <elog.h>

#include "cmd.h"


int
cmd_init(struct cmd *c, size_t size) {
    char *p;

    p = malloc(size);
    if (p == NULL) {
        return -1;
    }

    c->size = size;
    c->len = 0;
    c->buff = p;
    c->back = NULL;
    return 0;
}


void
cmd_deinit(struct cmd *c) {
    if (c->back) {
        free(c->back);
    }

    if (c->buff) {
        free(c->buff);
    }

    c->size = 0;
    c->len = 0;
    c->buff = NULL;
    c->back = NULL;
}


static int
_backup(struct cmd *c) {
    if (c->back) {
        return 0;
    }

    if (!c->len) {
        return 0;
    }

    c->back = malloc(c->len + 1);
    if (c->back == NULL) {
        return -1;
    }

    strncpy(c->back, c->buff, c->len);
    c->back[c->len] = 0;
    return 0;
}


void
cmd_commit(struct cmd *c) {
    if (c->back == NULL) {
    }

    free(c->back);
    c->back = NULL;
}


void
cmd_restore(struct cmd *c) {
    if (c->back == NULL) {
        return;
    }

    c->len = stpcpy(c->buff, c->back) - c->buff;
    free(c->back);
    c->back = NULL;
}


int
cmd_append(struct cmd *c, char ch) {
    if (cmd_isfull(c)) {
        return -1;
    }

    if (_backup(c)) {
        return -1;
    }

    c->buff[c->len++] = ch;
    return 0;
}


int
cmd_insert(struct cmd *c, char ch, int index) {
    /* 012345
     * abcde
     *    ^
     * abcXde
     */
    int i;

    if (cmd_isfull(c)) {
        return -1;
    }

    if (cmd_isempty(c)) {
        if (index) {
            return -1;
        }

        return cmd_append(c, ch);
    }

    /* backup */
    if (_backup(c)) {
        return -1;
    }

    /* shift right */
    for (i = c->len - 1; i >= index; i--) {
        c->buff[i + 1] = c->buff[i];
    }
    c->len++;
    c->buff[index] = ch;
    return 0;
}


int
cmd_set(struct cmd *c, char ch, int index) {
    /* backup */
    if (_backup(c)) {
        return -1;
    }

    c->buff[index] = ch;
    return 0;
}


int
cmd_delete(struct cmd *c, int index, int count) {
    int toindex;
    int i;

    if (!c->len || !count) {
        return -1;
    }

    /* backup */
    if (_backup(c)) {
        return -1;
    }

    if (index < 0) {
        index += c->len;
    }

    toindex = index + count;
    toindex = MAX(toindex, 0);
    toindex = MIN(toindex, c->len);

    if (toindex < index) {
        /* swap values to perform backward removal */
        index ^= toindex;
        toindex ^= index;
        index ^= toindex;
    }
    count = toindex - index;

    /* shift left */
    /* 012345
     * abcd
     *    ^
     * abc
     */
    for (i = index; i < (c->len - count); i++) {
        c->buff[i] = c->buff[i + count];
    }
    c->len -= count;

    return index;
}


int
cmd_copy(struct cmd *dst, struct cmd *src) {
    strncpy(dst->buff, src->buff, src->len);
    dst->len = src->len;
    return 0;
}
