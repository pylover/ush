#include <elog.h>

#include "ascii.h"
#include "term.h"
#include "vi.h"


#undef UAIO_ARG1
#undef UAIO_ARG2
#undef UAIO_ENTITY
#define UAIO_ENTITY vi
#include "uaio_generic.c"


static void
_nextwords(struct vi *vi, int words) {
    /*
     * b: blank
     * w: alphanum
     * s: everything else
     *
     * prev cur  action
     * w    w    i++; continue
     * *    b    i++; continue
     * *    *    found
     *
     */
    struct term *term = vi->term;
    struct cmd *cmd = TERM_CMDLINE(term);
    int i = term->col;
    char cur = 0;
    char prev = 0;

    cur = cmd_getc(cmd, i);
    while (words && (++i < cmd->len)) {
        prev = cur;
        cur = cmd_getc(cmd, i);
        if (!ASCII_ISBLANK(cur) &&
                (!ASCII_ISALPHANUM(cur) || !ASCII_ISALPHANUM(prev)) &&
                (i != term->col)) {
            term_cursor_move(term, i - term->col);
            words--;
        }
    }

    if (!(cmd->len - i) && words) {
        term_cursor_move(term, cmd->len - 1);
    }
}


static void
_nextendofwords(struct vi *vi, int words) {
    /*
     * b: blank
     * w: alphanum
     * s: everything else
     *
     * cur  next   action
     * w    w      i--; continue
     * b    *      i--; continue
     * *    *      found cur; i--; continue
     *
     */
    struct term *term = vi->term;
    struct cmd *cmd = TERM_CMDLINE(term);
    int i = term->col;
    char cur = 0;
    char next = 0;

    while (words && (++i < cmd->len)) {
        cur = cmd_getc(cmd, i);
        next = i > 0? cmd_getc(cmd, i + 1): 0;
        if (!ASCII_ISBLANK(cur) &&
                (!ASCII_ISALPHANUM(cur) || !ASCII_ISALPHANUM(next)) &&
                (i != term->col)) {
            term_cursor_move(term, i - term->col);
            words--;
        }
    }
}


static void
_prevwords(struct vi *vi, int words) {
    /*
     * b: blank
     * w: alphanum
     * s: everything else
     *
     * cur  next   action
     * w    w      i--; continue
     * b    *      i--; continue
     * *    *      found cur; i--; continue
     *
     */
    struct term *term = vi->term;
    struct cmd *cmd = TERM_CMDLINE(term);
    int i = term->col;
    char cur = 0;
    char next = 0;

    if ((i <= 0) || (cmd->len <= 0)) {
        return;
    }

    while (words && (--i >= 0)) {
        cur = cmd_getc(cmd, i);
        next = i > 0? cmd_getc(cmd, i - 1): 0;
        if (!ASCII_ISBLANK(cur) &&
                (!ASCII_ISALPHANUM(cur) || !ASCII_ISALPHANUM(next)) &&
                (i != term->col)) {
            term_cursor_move(term, i - term->col);
            words--;
        }
    }
}


void
vi_init(struct vi *vi, struct term *term) {
    vi->term = term;
    vi->repeat = -1;
    vi_swmode(vi, VI_INSERT);
}


void
vi_swmode(struct vi *vi, enum vi_mode mode) {
    if (vi->mode == mode) {
        return;
    }

    vi->mode = mode;
    if ((mode == VI_NORMAL) && vi->term->col) {
        term_cursor_move(vi->term, -1);
    }
}


ASYNC
viA(struct uaio_task *self, struct vi *vi) {
    char c;
    struct term *term = vi->term;
    struct cmd *cmdline = TERM_CMDLINE(term);
    UAIO_BEGIN(self);

aread:
    EUART_AREADT(self, &term->reader, 3, CONFIG_USH_TERM_READER_TIMEOUT_US);
    if (TERM_INBUFF_COUNT(term) < 1) {
        UAIO_RETURN(self);
    }

    c = TERM_INBUFF_GET(term);
    if (c == ASCII_LF) {
        vi_swmode(vi, VI_INSERT);
        UAIO_RETURN(self);
    }

    TERM_INBUFF_SKIP(term, 1);

    // DEBUG("VI: repeat: %d, c: %c", vi->repeat, c);
    if (vi->repeat == -1) {
        if (ASCII_IS1TO9(c)) {
            vi->repeat = c - '0';
            goto aread;
        }
    }
    else if (ASCII_ISDIGIT(c)) {
        vi->repeat *= 10;
        vi->repeat += c - '0';
        goto aread;
    }

    if (vi->repeat <= 0) {
        vi->repeat = 1;
    }
    // DEBUG("VI: repeat: %d", vi->repeat);

    if (c == 'd') {
        if (TERM_INBUFF_COUNT(term) < 1) {
            EUART_AREAD(self, &term->reader, 1);
        }
        c = TERM_INBUFF_POP(term);
        if (c == '$') {
            term_delete(term, cmdline->len);
        }
        else if (c == '0') {
            term_delete(term, -cmdline->len);
        }
        else if (c == 'd') {
            term_cursor_move(term, -term->col);
            term_delete(term, cmdline->len);
        }
        UAIO_RETURN(self);
    }

    if (c == 'r') {
        if (TERM_INBUFF_COUNT(term) < 1) {
            EUART_AREAD(self, &term->reader, 1);
        }
        c = TERM_INBUFF_POP(term);
        term_set(term, c);
        UAIO_RETURN(self);
    }

    switch (c) {
        case 'i':
            vi_swmode(vi, VI_INSERT);
            break;

        case 'k':
            if (!term_history_rotate(term, vi->repeat)) {
                term_cursor_move(term, -term->col);
            }
            break;

        case 'j':
            if (!term_history_rotate(term, -vi->repeat)) {
                term_cursor_move(term, -term->col);
            }
            break;

        case 'l':
            term_cursor_move(term, vi->repeat);
            break;

        case 'h':
            term_cursor_move(term, -vi->repeat);
            break;

        case '0':
            term_cursor_move(term, -term->col);
            break;

        case '$':
            term_cursor_move(term, cmdline->len);
            break;

        case 'a':
            vi_swmode(vi, VI_INSERT);
            term_cursor_move(term, 1);
            break;

        case 'A':
            vi_swmode(vi, VI_INSERT);
            term_cursor_move(term, cmdline->len);
            break;

        case 'x':
            term_delete(term, vi->repeat);
            break;

        case 'w':
            _nextwords(vi, vi->repeat);
            break;

        case 'b':
            _prevwords(vi, vi->repeat);
            break;

        case 'e':
            _nextendofwords(vi, vi->repeat);
            break;

        default:
            WARN("vi command: %c is not supported", c);
            break;
    }
    vi->repeat = -1;

    UAIO_FINALLY(self);
}
