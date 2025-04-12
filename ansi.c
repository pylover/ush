#include "term.h"


ASYNC
ansiA(struct uaio_task *self, struct term *term) {
    int skip = 0;
    char c = 0;
    UAIO_BEGIN(self);

    EUART_AREADT(self, &term->reader, 4, CONFIG_USH_TERM_READER_TIMEOUT_US);
    if (TERM_INBUFF_COUNT(term) < 3) {
        goto insufficient;
    }

    /* ansi control: [ */
    c = TERM_INBUFF_GETOFF(term, 1);
    if (c != '[') {
        goto notsupported;
    }

    /* skip ESC and [ */
    skip = 2;

    /* ansi command */
    c = TERM_INBUFF_GETOFF(term, 2);

    /* delete ^[3~ */
    if (c == '3') {
        if ((TERM_INBUFF_COUNT(term) <= 3) ||
                TERM_INBUFF_GETOFF(term, 3) != '~') {
            goto notsupported;
        }

        term_delete(term, 1);
        skip += 2;
        goto eat;
    }

    switch (c) {
        case 'A':
            term_history_rotate(term, 1);
            skip++;
            break;
        case 'B':
            term_history_rotate(term, -1);
            skip++;
            break;
        case 'C':
            term_cursor_move(term, 1);
            skip++;
            break;
        case 'D':
            term_cursor_move(term, -1);
            skip++;
            break;
        default:
            goto notsupported;
    }

    goto eat;


insufficient:
notsupported:
    UAIO_THROW2(self, EINVAL);

eat:
    TERM_INBUFF_SKIP(term, skip);
    UAIO_CLEARERROR(self);
    UAIO_FINALLY(self);
}
