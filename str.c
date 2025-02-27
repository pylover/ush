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
