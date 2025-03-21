#include <stdlib.h>

#include <elog.h>

#include "cmd.h"


int
cmd_init(struct cmd *s, size_t size) {
    char *p;

    p = malloc(size);
    if (p == NULL) {
        return -1;
    }

    s->size = size;
    s->len = 0;
    s->buff = p;
    return 0;
}


void
cmd_deinit(struct cmd *s) {
    if (s->buff) {
        free(s->buff);
    }
}


int
cmd_append(struct cmd *s, char c) {
    if (cmd_isfull(s)) {
        return -1;
    }

    s->buff[s->len++] = c;
    return 0;
}


int
cmd_delete(struct cmd *s, int index) {
    int last;
    char *buf;

    if (s->len == 0) {
        return -1;
    }

    if (index < 0) {
        index += s->len;
    }

    last = s->len - 1;
    if ((index < 0) || (index > last)) {
        return -1;
    }

    /* 012345
     * abcde
     *    ^
     * abce
     */
    s->len--;
    buf = s->buff;
    for (; index < last; index++) {
        buf[index] = buf[index + 1];
    }

    return 0;
}


int
cmd_copy(struct cmd *dst, struct cmd *src) {
    strncpy(dst->buff, src->buff, src->len);
    dst->len = src->len;
    return 0;
}
