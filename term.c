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
#include "uaio_generic.c"


#ifdef CONFIG_USH_VI

#define vi_inserting(t) ((t)->mode == VI_INSERT)

static void
_vi_switch(struct term *term, enum vi_mode mode) {
    if (term->mode == mode) {
        return;
    }
    DEBUG("VI: Switched to %s mode", mode == VI_INSERT? "insert": "normal");
    term->mode = mode;
}

#endif  // CONFIG_USH_VI


static int
_printf(struct term *term, const char *restrict fmt, ...) {
    int ret;
    va_list args;

    va_start(args, fmt);
    ret = vdprintf(term->outfd, fmt, args);
    va_end(args);
    return ret;
}


static void
_cursor_move(struct term *term, int cols) {
    int newcol = term->col + cols;
    if ((newcol < 0) || (newcol > TERM_CMDLINE(term)->len) ||
            (newcol == term->col)) {
        return;
    }

    _printf(term, "%c[%d%c", ASCII_ESC, abs(cols), cols < 0? 'D': 'C');
    term->col = newcol;
}


static int
_insert(struct term *term, char c) {
    // 01234567
    // abcdefgh
    //    ^
    // abc defgh
    //    ^
    int dirty;
    struct cmd *cmd = TERM_CMDLINE(term);

    if (cmd_insert(cmd, c, term->col)) {
        return -1;
    }

    write(term->outfd, &c, 1);
    term->col++;

    dirty = cmd->len - term->col;
    if (dirty) {
        write(term->outfd, cmd_ptroff(cmd, term->col), dirty);
        _printf(term, "%c[%dD", ASCII_ESC, abs(dirty));
    }

    return 0;
}


static void
_delete(struct term *term) {
    /*   buffer     terminal
     *   01234567   01234567
     * 0 abcdefgh   abcdefgh
     *       ^          ^
     * 1 abcdfgh    abcdefgh
     *       ^          ^
     * 2 abcdfgh    abcdfghh
     *       ^             ^
     * 3 abcdfgh    abcdfgh
     *       ^             ^
     * 4 abcdfgh    abcdfgh
     *       ^          ^
     *
     *   012   012
     * 0 abc   abc
     *   ^     ^
     */
    /* 0 */
    int curoff;
    struct cmd *cmd = TERM_CMDLINE(term);

    /* 1 */
    if (cmd_delete(cmd, term->col)) {
        return;
    }

    /* 2 */
    curoff = cmd->len - term->col;
    if (curoff) {
        write(term->outfd, cmd_ptroff(cmd, term->col), curoff);
    }

    write(term->outfd, " \b", 2);
    if (!curoff) {
        return;
    }

    /* 3 */
    write(term->outfd, " \b", 2);

    /* 4 */
    _printf(term, "%c[%dD", ASCII_ESC, abs(curoff));
}


static void
_backspace(struct term *term) {
    /*   buffer     terminal
     *   01234567   01234567
     * 0 abcdefgh   abcdefgh
     *       ^          ^
     * 1 abcefgh    abcdefgh
     *      ^           ^
     * 2 abcefgh    abcefghh
     *      ^              ^
     * 3 abcefgh    abcefgh
     *      ^              ^
     * 4 abcefgh    abcefgh
     *      ^          ^
     *
     *   0123   0123
     * 0 abc    abc
     *      ^      ^
     * 1 ab     abc
     *     ^       ^
     * 2 ab     abc
     *     ^      ^
     * 3 ab     ab
     *     ^      ^
     */
    /* 0 */
    int curoff;
    struct cmd *cmd = TERM_CMDLINE(term);

    if (!term->col) {
        return;
    }

    /* 1 */
    if (cmd_delete(cmd, term->col - 1)) {
        return;
    }
    term->col--;

    /* 2 */
    write(term->outfd, "\b", 1);
    curoff = cmd->len - term->col;
    if (curoff) {
        write(term->outfd, cmd_ptroff(cmd, term->col), curoff);
    }

    /* 3 */
    write(term->outfd, " \b", 2);

    /* 4 */
    if (curoff) {
        _printf(term, "%c[%dD", ASCII_ESC, abs(curoff));
    }
}


static void
_rewrite(struct term *term) {
    struct cmd *cmd = TERM_CMDLINE(term);

    if (term->col) {
        _cursor_move(term, -term->col);
    }

    _printf(term, "%s%.*s", ANSI_ERASETOEND, cmd->len, cmd_ptr(cmd));
    term->col = cmd->len;
}


static int
_history_rotate(struct term *term, int steps) {
    int r;

    r = term->rotation + steps;
    if ((r < 0) || (r > TERM_HISTORY_COUNT(term))) {
        return -1;
    }

    cmd_commit(TERM_CMDLINE(term));
    term->rotation += steps;
    _rewrite(term);
    return 0;
}


/** Puts the current command line (cmdring's head) into the history and
 * initialize the new head as the command line
 */
static int
_history_put(struct term *term) {
    struct cmd *first = NULL;
    struct cmd *cmdline = TERM_CMDLINE(term);

    if (cmd_isempty(cmdline)) {
        goto done;
    }

    /* prevent duplicate entries in order */
    if (TERM_HISTORY_COUNT(term)) {
        first = TERM_HISTORY_OFFSET(term, 1);
        if (cmd_isempty(first) || (cmd_compare(first, cmdline) == 0)) {
            goto done;
        }
    }

    /* reuse the last item if possible */
    if (TERM_HISTORY_ISFULL(term)) {
        TERM_HISTORY_SHRINK(term);
        TERM_HISTORY_EXTEND(term);
        goto done;
    }

    /* initialize and allocate the first item of the history (aka cmdline) */
    TERM_HISTORY_EXTEND(term);
    if (cmd_init(TERM_HISTORY_OFFSET(term, 0), CONFIG_USH_TERM_LINESIZE)) {
        TERM_HISTORY_EXTEND(term);
        return -1;
    }

done:
    TERM_HISTORY_OFFSET(term, 0)->len = 0;
    term->col = 0;
    return 0;
}


static int
_prompt(struct term *term) {
    if (_printf(term, "%s%s%s:# ", LINEBREAK, ANSI_RESET,
                CONFIG_USH_PROMPT) == -1) {
        return -1;
    }

    term->rotation = 0;
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
    if (cmdring_init(&term->history,
                CONFIG_USH_TERM_HISTORY_RINGMASK_BITS)) {
        goto rollback;
    }

    /* initialize and allocate the first item of the history */
    if (cmd_init(TERM_HISTORY_OFFSET(term, 0), CONFIG_USH_TERM_LINESIZE)) {
        goto rollback;
    }

    term->outfd = outfd;
    term->rotation = 0;
    term->col = 0;
#ifdef CONFIG_USH_VI
    _vi_switch(term, VI_INSERT);
#endif  // CONFIG_USH_VI

    return 0;

rollback:
    cmdring_deinit(&term->history);
    euart_reader_deinit(&term->reader);
    return -1;
}


/** Deinitialize and release all resource allocated for the terminal
 */
int
term_deinit(struct term *term) {
    int ret = 0;
    struct cmdring *history = &term->history;

    /* flush the cmdring */
    cmd_deinit(ERING_HEADPTR(history));
    while (TERM_HISTORY_COUNT(term)) {
        cmd_deinit(ERING_TAILPTR(history));
        ERING_INCRTAIL(history);
    }

    ret |= cmdring_deinit(history);
    ret |= euart_reader_deinit(&term->reader);
    return ret;
}


ASYNC
_viA(struct uaio_task *self, struct term *term) {
    char c;
    UAIO_BEGIN(self);
    EUART_AREADT(self, &term->reader, 3, CONFIG_USH_TERM_READER_TIMEOUT_US);
    if (!TERM_INBUFF_COUNT(term)) {
        UAIO_RETURN(self);
    }

    c = TERM_INBUFF_GET(term);
    if (c == ASCII_LF) {
        _vi_switch(term, VI_INSERT);
        UAIO_RETURN(self);
    }

    TERM_INBUFF_SKIP(term, 1);
    switch (c) {
        case 'i':
            _vi_switch(term, VI_INSERT);
            break;
        case 'k':
            if (!_history_rotate(term, 1)) {
                _cursor_move(term, -term->col);
            }
            break;
        case 'j':
            if (!_history_rotate(term, -1)) {
                _cursor_move(term, -term->col);
            }
            break;
        case 'l':
            _cursor_move(term, 1);
            break;
        case 'h':
            _cursor_move(term, -1);
            break;
        default:
            WARN("vi command: %c is not supported", c);
            break;
    }

    UAIO_FINALLY(self);
}


static ASYNC
_escape(struct uaio_task *self, struct term *term) {
    char c = 0;
    UAIO_BEGIN(self);

    EUART_AREADT(self, &term->reader, 3, CONFIG_USH_TERM_READER_TIMEOUT_US);
    if (TERM_INBUFF_COUNT(term) < 2) {
        UAIO_RETURN(self);
    }
    c = TERM_INBUFF_POP(term);

    /* ansi control */
    if (c != '[') {
        WARN("escape sequence not supported: %d", c);
        UAIO_RETURN(self);
    }

    if (TERM_INBUFF_COUNT(term) == 0) {
        UAIO_RETURN(self);
    }
    c = TERM_INBUFF_POP(term);

    /* delete */
    if (c == '3') {
        if (TERM_INBUFF_COUNT(term) == 0) {
            UAIO_RETURN(self);
        }
        c = TERM_INBUFF_POP(term);
        if (c == 126) {
            _delete(term);
            UAIO_RETURN(self);
        }
        WARN("escape sequence not supported: ^[3%d", c);
        UAIO_RETURN(self);
    }

    switch (c) {
        case 'A':
            _history_rotate(term, 1);
            break;
        case 'B':
            _history_rotate(term, -1);
            break;
        case 'C':
            _cursor_move(term, 1);
            break;
        case 'D':
            _cursor_move(term, -1);
            break;
        default:
            WARN("escape sequence: %c is not supported", c);
            break;
    }


    UAIO_FINALLY(self);
}


ASYNC
term_readA(struct uaio_task *self, struct term *term) {
    char c;
    struct cmd *cmd;
    UAIO_BEGIN(self);

prompt:
    _history_put(term);
    _prompt(term);

    while (true) {
        while (TERM_INBUFF_COUNT(term)) {
            c = TERM_INBUFF_GET(term);

            //DEBUG("c: %d", c);

            /* ansi */
            if (c == ASCII_ESC) {
                TERM_INBUFF_SKIP(term, 1);
#ifdef CONFIG_USH_VI
                if (term->mode == VI_INSERT) {
                    _vi_switch(term, VI_NORMAL);
                    continue;
                }
#endif  // CONFIG_USH_VI
                TERM_AWAIT(self, _escape, term);
                continue;
            }

#ifdef CONFIG_USH_VI
            /* vi */
            if (term->mode == VI_NORMAL) {
                TERM_AWAIT(self, _viA, term);
                continue;
            }
#endif  // CONFIG_USH_VI

            /* No need to more examination, delete the char from buffer */
            TERM_INBUFF_SKIP(term, 1);

            /* backspace */
            if (ASCII_ISBACKSPACE(c)) {
                _backspace(term);
                continue;
            }

            /* enter */
            if (c == ASCII_LF) {
                cmd = TERM_CMDLINE(term);
                if (cmd->len == 0) {
                    goto prompt;
                }
                if (term->rotation) {
                    term->rotation = 0;
                    cmd_copy(TERM_CMDLINE(term), cmd);
                    cmd_restore(cmd);
                }
                UAIO_RETURN(self);
            }

            if (_insert(term, c)) {
                UAIO_THROW2(self, ENOBUFS);
            }
        }
        fsync(term->outfd);
        EUART_AREAD(self, &term->reader, 1);
    }

    UAIO_FINALLY(self);
}
