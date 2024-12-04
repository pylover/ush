#include <stdarg.h>

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
term_backspace(struct ush *sh) {
    //   buff       term
    //   01234567   01234567
    // 1 abcdefgh   abcdefgh
    //       ^          ^
    // 2 abcdefgh   abc efgh
    //       ^         ^
    // 3 abcefgh    abc efgh
    //      ^          ^
    // 4 abcefgh    abcefgh
    //          ^          ^
    if (!sh->cursor) {
        return;
    }

    // (2)
    putchar('\b');

    // (3)
    for (int i = sh->cursor; i < sh->cmdsize; i++) {
        sh->cmdline[i - 1] = sh->cmdline[i];
    }
    sh->cursor--;
    sh->cmdsize--;

    // (4)
    int curoff = sh->cmdsize - sh->cursor;
    if (curoff) {
        term_overwrite(sh, "%.*s", curoff, sh->cmdline + sh->cursor);
    }
    printf(" \b");
    term_navleft(sh, curoff);
}


void
term_navleft(struct ush *sh, int c) {
    if (!sh->cursor) {
        return;
    }

    if (!c) {
        return;
    }

    printf("%c[%dD", CHAR_ESCAPE, c);
    sh->cursor -= c;
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


int
term_insert(struct ush *sh, char c) {
    // 01234567
    // abcdefgh
    //    ^
    // abc defgh
    //    ^
    int curoff = sh->cmdsize - sh->cursor;

    if (sh->cmdsize == CONFIG_USH_CMDLINE_MAX) {
        return -1;
    }

    if (curoff) {
        for (int i = sh->cmdsize - 1; i >= sh->cursor; i--) {
            sh->cmdline[i + 1] = sh->cmdline[i];
        }
    }

    sh->cmdline[sh->cursor] = c;
    sh->cmdsize++;
    putchar(c);
    sh->cursor++;

    if (curoff) {
        term_overwrite(sh, "%.*s", curoff, sh->cmdline + sh->cursor);
        term_navleft(sh, curoff);
    }

    return 0;
}


void
term_navend(struct ush *sh) {
    term_navright(sh, sh->cmdsize - sh->cursor);
}


void
term_rewrite(struct ush *sh) {
    if (sh->cursor) {
        term_navleft(sh, sh->cursor);
    }
    printf("\33[K");
    term_overwrite(sh, "%.*s", sh->cmdsize, sh->cmdline);
}
