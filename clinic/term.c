#include <stdarg.h>

#include <elog.h>

#include "term.h"
#include "char.h"


int
term_overwrite(struct ush *sh, const char *restrict fmt, ...) {
    va_list args;

    va_start(args, fmt);
    int ret = vprintf(fmt, args);
    va_end(args);

    sh->cursor += ret;
    return ret;
}


void
term_navright(struct ush *sh, int c) {
    if (sh->cursor == sh->cmdsize) {
        return;
    }

    if (!c) {
        return;
    }

    printf("%c[%dC", CHAR_ESCAPE, c);
    sh->cursor += c;
}


void
term_navend(struct ush *sh) {
    term_navright(sh, sh->cmdsize - sh->cursor);
}


void
term_rewrite(struct ush *sh) {
    if (sh->cursor) {
        term_navleft(&sh->cmdline, sh->cursor);
    }
    printf("\33[K");
    term_overwrite(sh, "%.*s", sh->cmdsize, sh->cmdline);
}
