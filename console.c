#include <string.h>

#include <elog.h>

#include "ascii.h"
#include "ansi.h"
#include "console.h"


#undef UAIO_ARG1
#undef UAIO_ARG2
#undef UAIO_ENTITY
#define UAIO_ENTITY console
#include "uaio_generic.c"


int
console_init(struct console *con, struct euart_device *device) {
    if ((con == NULL) || (device == NULL)) {
        return -1;
    }

    if (euart_reader_init(&con->reader, device,
                CONFIG_USH_CONSOLE_BUFFMASK_BITS)) {
        return -1;
    }

    // if (history_init(&con->reader
    return 0;
}


int
console_deinit(struct console *con) {
    if (con == NULL) {
        return -1;
    }

    if (euart_reader_deinit(&con->reader)) {
        return -1;
    }

    return 0;
}


int
console_printf(struct console *con, const char *restrict fmt, ...) {
    int ret;
    va_list args;

    va_start(args, fmt);
    ret = vdprintf(con->reader.device->outfd, fmt, args);
    va_end(args);
    return ret;
}


static ASYNC
_escape(struct uaio_task *self, struct console *con) {
    char c;
    struct euart_reader *reader = &con->reader;
    struct u8ring *ring = &reader->ring;
    UAIO_BEGIN(self);
    ERING_SKIP(ring, 1);
    DEBUG("ansiA");

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
console_readA(struct uaio_task *self, struct console *con) {
    char c;
    struct euart_reader *reader = &con->reader;
    struct u8ring *ring = &reader->ring;
    UAIO_BEGIN(self);

    con->linesize = 0;
    while (true) {
        if (con->linesize >= CONFIG_USH_CONSOLE_LINESIZE) {
            UAIO_THROW2(self, ENOBUFS);
        }
        while (ERING_USED(ring)) {
            c = ERING_GET(ring);

            if (c == ASCII_ESC) {
                CONSOLE_AWAIT(self, _escape, con);
                continue;
            }

            write(reader->device->outfd, &c, 1);

            /* consume the char */
            ERING_SKIP(ring, 1);

            if (c == ASCII_LF) {
                UAIO_RETURN(self);
            }
            con->line[con->linesize++] = c;
        }
        EUART_AREAD(self, reader, 1);
    }

    UAIO_FINALLY(self);
}
// DEBUGN("reader: status: %d count: %d -- ", reader->status, used);
// int w = u8ring_writeout(ring, elog_outfd, used);
// PRINT(" -- w: %d\n", w);
// ERING_SKIP(ring, used);
