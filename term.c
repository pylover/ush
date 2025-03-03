#include <string.h>

#include <elog.h>

#include "config.h"
#include "ascii.h"
#include "ansi.h"
#include "term.h"


#undef ERING_PREFIX
#define ERING_PREFIX cmd
#include <ering.c>


#undef UAIO_ARG1
#undef UAIO_ARG2
#undef UAIO_ENTITY
#define UAIO_ENTITY term
#define UAIO_ARG1 struct cmd*
#include "uaio_generic.c"


#define CMDLINE(t) ERING_HEADPTROFF(&(t)->cmdring, (t)->row)


static int
_printf(struct term *term, const char *restrict fmt, ...) {
    int ret;
    va_list args;

    va_start(args, fmt);
    ret = vdprintf(term->outfd, fmt, args);
    va_end(args);
    return ret;
}


static int
_cmdline_append(struct term *term, char c) {
    struct cmd *s = CMDLINE(term);

    if (cmd_append(s, c)) {
        return -1;
    }

    if (write(term->outfd, &c, 1) == -1) {
        cmd_delete(s, -1);
        ERROR("write");
        return -1;
    }

    return 0;
}


// static int
// _cmdline_rewrite(struct term *term) {
//     if (sh->cursor) {
//         term_navleft(&sh->cmdline, sh->cursor);
//     }
//     printf("\33[K");
//     term_overwrite(sh, "%.*s", sh->cmdsize, sh->cmdline);
// }
//
//
// static int
// _cmdline_nav(struct term *term, int step) {
//     if (!l->cursor) {
//         return;
//     }
//
//     if (!c) {
//         return;
//     }
//
//     printf("%c[%dD", CHAR_ESCAPE, c);
//     l->cursor -= c;
// }
//
//
// static int
// _history_nav(struct term *term, int step) {
//     struct cmdring *history = &term->history;
//
//     if (cmdring_prev(history)) {
//         return -1;
//     }
//
//     return -1;
// }
//
//
// static int
// _row_push(struct term *term) {
//     struct cmdring *cr = &term->history;
//     if (ERING_ISFULL(cr)) {
//         ERING_TAILPTR(cr)
//         ERING_DECR(cr);
//     }
// }


static int
_prompt(struct term *term) {
    if (_printf(term, "%s%s%s:# ", LINEBREAK, ANSI_RESET,
                CONFIG_USH_PROMPT) == -1) {
        return -1;
    }

    return 0;
}


/** Initialize and allocate resources for the terminal
 */
int
term_init(struct term *term, int infd, int outfd) {
    if ((term == NULL) || (infd < 0) || (outfd < 0)) {
        return -1;
    }

    /* zero set */
    memset(term, 0, sizeof(struct term));

    /* initialize uart reader */
    if (euart_reader_init(&term->reader, infd,
                CONFIG_USH_TERM_READER_RINGMASK_BITS)) {
        return -1;
    }

    /* initialize commands circular buffer */
    if (cmdring_init(&term->cmdring,
                CONFIG_USH_TERM_HISTORY_RINGMASK_BITS)) {
        goto rollback;
    }

    /* initialize and allocate the first item of the cmdring */
    if (cmd_init(ERING_HEADPTR(&term->cmdring), CONFIG_USH_TERM_LINESIZE)) {
        goto rollback;
    }

    term->outfd = outfd;
    term->row = 0;
    term->col = 0;
    return 0;

rollback:
    cmdring_deinit(&term->cmdring);
    euart_reader_deinit(&term->reader);
    return -1;
}


/** Deinitialize and release all resource allocated for the terminal
 */
int
term_deinit(struct term *term) {
    int ret = 0;
    struct cmdring *cr = &term->cmdring;

    /* flush the cmdring */
    cmd_deinit(ERING_HEADPTR(cr));
    while (ERING_USED(cr)) {
        cmd_deinit(ERING_TAILPTR(cr));
        ERING_DECR(cr);
    }

    ret |= cmdring_deinit(cr);
    ret |= euart_reader_deinit(&term->reader);
    return ret;
}


static ASYNC
_escape(struct uaio_task *self, struct term *term, struct cmd *out) {
    char c;
    struct euart_reader *reader = &term->reader;
    struct u8ring *input = &reader->ring;
    UAIO_BEGIN(self);
    ERING_SKIP(input, 1);

    EUART_AREAD(self, reader, 1);
    c = ERING_POP(input);
    if (c != '[') {
        WARN("escape sequence not supported: %d", c);
        UAIO_RETURN(self);
    }

    EUART_AREAD(self, reader, 1);
    c = ERING_POP(input);
    DEBUG("escape sequence: %c", c);
    switch (c) {
        case 'A':
            // term_history_nav(term, 1);
        case 'B':
            // term_history_nav(term, -1);
        case 'C':
        case 'D':
    }

    UAIO_FINALLY(self);
}


ASYNC
term_readA(struct uaio_task *self, struct term *term, struct cmd *out) {
    char c;
    struct euart_reader *reader = &term->reader;
    struct u8ring *input = &reader->ring;
    UAIO_BEGIN(self);

    _prompt(term);
    fflush(stdout);

    while (true) {
        while (ERING_USED(input)) {
            c = ERING_GET(input);

            /* ansi */
            if (c == ASCII_ESC) {
                TERM_AWAIT(self, _escape, term, NULL);
                continue;
            }

            /* consume the char */
            ERING_SKIP(input, 1);

            if (c == ASCII_LF) {
                cmd_copy(out, CMDLINE(term));
                UAIO_RETURN(self);
            }

            if (_cmdline_append(term, c)) {
                UAIO_THROW2(self, ENOBUFS);
            }

        }
        EUART_AREAD(self, reader, 1);
    }

    UAIO_FINALLY(self);
}
