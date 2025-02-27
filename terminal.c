#include <string.h>

#include <elog.h>

#include "ascii.h"
#include "ansi.h"
#include "terminal.h"


#undef UAIO_ARG1
#undef UAIO_ARG2
#undef UAIO_ENTITY
#define UAIO_ENTITY terminal
#include "uaio_generic.c"


int
terminal_init(struct terminal *ter, struct euart_device *device) {
    if ((ter == NULL) || (device == NULL)) {
        return -1;
    }

    if (euart_reader_init(&ter->reader, device,
                CONFIG_USH_TERMINAL_BUFFMASK_BITS)) {
        return -1;
    }

    // if (history_init(&ter->reader
    return 0;
}


int
terminal_deinit(struct terminal *ter) {
    if (ter == NULL) {
        return -1;
    }

    if (euart_reader_deinit(&ter->reader)) {
        return -1;
    }

    return 0;
}


int
terminal_printf(struct terminal *ter, const char *restrict fmt, ...) {
    int ret;
    va_list args;

    va_start(args, fmt);
    ret = vdprintf(ter->reader.device->outfd, fmt, args);
    va_end(args);
    return ret;
}


static ASYNC
_escape(struct uaio_task *self, struct terminal *ter) {
    char c;
    struct euart_reader *reader = &ter->reader;
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
terminal_readA(struct uaio_task *self, struct terminal *ter) {
    char c;
    struct euart_reader *reader = &ter->reader;
    struct u8ring *ring = &reader->ring;
    UAIO_BEGIN(self);

    ter->linesize = 0;
    while (true) {
        if (ter->linesize >= CONFIG_USH_TERMINAL_LINESIZE) {
            UAIO_THROW2(self, ENOBUFS);
        }
        while (ERING_USED(ring)) {
            c = ERING_GET(ring);

            if (c == ASCII_ESC) {
                TERMINAL_AWAIT(self, _escape, ter);
                continue;
            }

            write(reader->device->outfd, &c, 1);

            /* consume the char */
            ERING_SKIP(ring, 1);

            if (c == ASCII_LF) {
                UAIO_RETURN(self);
            }
            ter->line[ter->linesize++] = c;
        }
        EUART_AREAD(self, reader, 1);
    }

    UAIO_FINALLY(self);
}
// DEBUGN("reader: status: %d count: %d -- ", reader->status, used);
// int w = u8ring_writeout(ring, elog_outfd, used);
// PRINT(" -- w: %d\n", w);
// ERING_SKIP(ring, used);
