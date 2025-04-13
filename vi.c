#include <elog.h>

#include "ascii.h"
#include "term.h"
#include "vi.h"


#undef UAIO_ARG1
#undef UAIO_ARG2
#undef UAIO_ENTITY
#define UAIO_ENTITY vi
#include "uaio_generic.c"


void
vi_init(struct vi *vi, struct term *term) {
    vi->term = term;
    vi->repeat = -1;
    vi_swmode(vi, VI_INSERT);
}


void
vi_swmode(struct vi *vi, enum vi_mode mode) {
    int col;

    if (vi->mode == mode) {
        return;
    }

    col = vi->term->col;
    DEBUG("VI: Switched to %s mode, col: %d",
            mode == VI_INSERT? "insert": "normal", col);
    vi->mode = mode;
    if ((mode == VI_NORMAL) && col) {
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

    DEBUG("VI: repeat: %d, c: %c", vi->repeat, c);
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
    DEBUG("VI: repeat: %d", vi->repeat);

    if (c == 'd') {
        if (TERM_INBUFF_COUNT(term) < 1) {
            EUART_AREAD(self, &term->reader, 1);
        }
        c = TERM_INBUFF_GET(term);
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
            term_cursor_nextwords(term, vi->repeat);
            break;

        default:
            WARN("vi command: %c is not supported", c);
            break;
    }
    vi->repeat = -1;

    UAIO_FINALLY(self);
}


