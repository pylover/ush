#include <string.h>

#include <elog.h>

#include "config.h"
#include "ascii.h"
#include "ansi.h"
#include "term.h"


#undef UAIO_ARG1
#undef UAIO_ARG2
#undef UAIO_ENTITY
#define UAIO_ENTITY term
#define UAIO_ARG1 struct str*
#include "uaio_generic.c"


int
term_init(struct term *term, struct euart_device *device) {
    if ((term == NULL) || (device == NULL)) {
        return -1;
    }

    if (euart_reader_init(&term->reader, device->infd,
                CONFIG_USH_TERM_READER_RINGMASK_BITS)) {
        return -1;
    }

    if (cmdring_init(&term->history,
                CONFIG_USH_TERM_HISTORY_RINGMASK_BITS)) {
        euart_reader_deinit(&term->reader);
        return -1;
    }

    term->device = device;
    return 0;
}


int
term_deinit(struct term *term) {
    int ret = 0;
    if (term == NULL) {
        return -1;
    }

    cmdring_flush(&term->history);
    ret |= cmdring_deinit(&term->history);
    ret |= euart_reader_deinit(&term->reader);
    return ret;
}


int
term_printf(struct term *term, const char *restrict fmt, ...) {
    int ret;
    va_list args;

    va_start(args, fmt);
    ret = vdprintf(TERM_OUTFD(term), fmt, args);
    va_end(args);
    return ret;
}


int
term_append(struct term *term, char c) {
    struct str *s = CMDRING_CURRENT(&term->history);

    if (str_append(s, c)) {
        return -1;
    }

    if (write(TERM_OUTFD(term), &c, 1) == -1) {
        str_delete(s, -1);
        ERROR("device write, fd: %d", TERM_OUTFD(term));
        return -1;
    }

    return 0;
}


int
term_prompt(struct term *term) {
    if (cmdring_pushnew(&term->history)) {
        return -1;
    }

    if (term_printf(term, "%s%s%s:# ", LINEBREAK, ANSI_RESET,
                CONFIG_USH_PROMPT) == -1) {
        return -1;
    }

    return 0;
}


static ASYNC
_escape(struct uaio_task *self, struct term *term, struct str *out) {
    char c;
    struct euart_reader *reader = &term->reader;
    struct u8ring *ring = &reader->ring;
    UAIO_BEGIN(self);
    ERING_SKIP(ring, 1);

    EUART_AREAD(self, reader, 1);
    c = ERING_POP(ring);
    if (c != '[') {
        WARN("escape sequence not supported: %d", c);
        UAIO_RETURN(self);
    }

    EUART_AREAD(self, reader, 1);
    c = ERING_POP(ring);
    DEBUG("escape sequence: %c", c);
    switch (c) {
        case 'A':
        case 'B':
        case 'C':
        case 'D':
    }

    UAIO_FINALLY(self);
}


ASYNC
term_readA(struct uaio_task *self, struct term *term, struct str *out) {
    char c;
    struct euart_reader *reader = &term->reader;
    struct u8ring *ring = &reader->ring;
    UAIO_BEGIN(self);

    term_prompt(term);
    fflush(stdout);

    while (true) {
        while (ERING_USED(ring)) {
            c = ERING_GET(ring);

            if (c == ASCII_ESC) {
                TERM_AWAIT(self, _escape, term, NULL);
                continue;
            }

            /* consume the char */
            ERING_SKIP(ring, 1);

            if (c == ASCII_LF) {
                str_copy(out, CMDRING_CURRENT(&term->history));
                UAIO_RETURN(self);
            }

            if (term_append(term, c)) {
                UAIO_THROW2(self, ENOBUFS);
            }

        }
        EUART_AREAD(self, reader, 1);
    }

    UAIO_FINALLY(self);
}
