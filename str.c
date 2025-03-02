#include <stdlib.h>
#include <string.h>

#include <elog.h>

#include "str.h"


#undef ERING_PREFIX
#define ERING_PREFIX str
#include <ering.c>


int
str_init(struct str *s, size_t size) {
    char *p;

    p = malloc(size);
    if (p == NULL) {
        return -1;
    }

    s->size = size;
    s->len = 0;
    s->start = p;
    return 0;
}


void
str_deinit(struct str *s) {
    if (s->start) {
        free(s->start);
    }
}


int
str_append(struct str *s, char c) {
    if (STR_FULL(s)) {
        return -1;
    }

    s->start[s->len++] = c;
    return 0;
}


int
str_delete(struct str *s, int index) {
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
    buf = s->start;
    for (; index < last; index++) {
        buf[index] = buf[index + 1];
    }

    return 0;
}


int
str_copy(struct str *dst, struct str *src) {
    strncpy(dst->start, src->start, src->len);
    dst->len = src->len;
    return 0;
}
