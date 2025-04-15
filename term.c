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
    if (term_printf(term, "%s%s%s:# ", LINEBREAK, ANSI_RESET,
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

#ifdef CONFIG_USH_VI
    vi_init(&term->vi, term);
#endif  // CONFIG_USH_VI

    term->outfd = outfd;
    term->rotation = 0;
    term->col = 0;
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


int
term_printf(struct term *term, const char *restrict fmt, ...) {
    int ret;
    va_list args;

    va_start(args, fmt);
    ret = vdprintf(term->outfd, fmt, args);
    va_end(args);
    return ret;
}


void
term_cursor_move(struct term *term, int cols) {
    int newcol = term->col + cols;
    int maxcol = TERM_CMDLINE(term)->len;
    int steps;

#ifdef CONFIG_USH_VI
    if (vi_mode(&term->vi) == VI_NORMAL) {
        maxcol = MAX(0, maxcol - 1);
    }
#endif  // CONFIG_USH_VI

    if (newcol < 0) {
        newcol = 0;
    }
    else if (newcol > maxcol) {
        newcol = maxcol;
    }

    steps = newcol - term->col;
    if (!steps) {
        return;
    }

    term_printf(term, "%c[%d%c", ASCII_ESC, abs(steps), steps < 0? 'D': 'C');
    term->col = newcol;
}


void
term_cursor_nextwords(struct term *term, int words) {
    /*
     * prev cur  next   action
     * *    w    w      i--; continue
     * *    b    *      i--; continue
     * w    w    sbn    found cur; i--; continue
     * *    s    wsbn   found cur; i--; continue
     *
     * status char  action
     * w      w     ++ continue
     * w      b     found
     * w      s     found
     * s      w     found
     * s      b     found
     * s      s     found
     *
     *
     * found:
     * 1. skip blank chars
     * 2. move cursor
     */
    struct cmd *cmd = TERM_CMDLINE(term);
    char status;
    char c;
    int i = term->col;

    if (!cmd->len) {
        return;
    }

    c = cmd_getc(cmd, i++);
    status = ASCII_ISALPHA(c)? 'w': 's';
    while (words && (i < cmd->len)) {
        c = cmd_getc(cmd, i);
        if (ASCII_ISALPHA(c)) {
            if (status == 'w') {
                i++;
                continue;
            }
            status = 'w';
        }
        else if (status == 'w') {
            status = 's';
        }

        /* skip blank chars */
        if (ASCII_ISBLANK(c)) {
            for (++i; i < cmd->len; i++) {
                if (!ASCII_ISBLANK(cmd_getc(cmd, i))) {
                    break;
                }
            }

            if (i >= cmd->len) {
                break;
            }
        }

        term_cursor_move(term, i - term->col);
        words--;
        i++;
    }

    if (!(cmd->len - i) && words) {
        term_cursor_move(term, cmd->len - 1);
    }
}


void
term_cursor_prevwords(struct term *term, int words) {
    /*
     * prev cur  next   action
     * *    w    w      i--; continue
     * *    b    *      i--; continue
     * w    w    sbn    found cur; i--; continue
     * *    s    wsbn   found cur; i--; continue
     *
     * found:
     * 1. skip blank chars
     * 2. move cursor
     */
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


int
term_insert(struct term *term, char c) {
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
        term_printf(term, "%c[%dD", ASCII_ESC, abs(dirty));
    }

    return 0;
}


void
term_rewrite(struct term *term, int index) {
    struct cmd *cmd = TERM_CMDLINE(term);

    index = MAX(0, index);
    index = MIN(cmd->len - 1, index);

    if (term->col != index) {
        term_cursor_move(term, index - term->col);
    }

    DEBUG("index; %d, cmdlen: %d, %.*s", index, cmd->len,
            cmd->len, cmd_ptr(cmd));
    term_printf(term, "%s%.*s", ANSI_ERASETOEND, (cmd->len - index),
            cmd_ptroff(cmd, index));
    term->col = cmd->len;
}


void
term_delete(struct term *term, unsigned int count) {
    /*   buffer     terminal
     *   01234567   01234567
     * 0 abcdefgh   abcdefgh   _delete(2)
     *       ^          ^
     * 1 abcdgh     abcdefgh
     *       ^          ^
     * 2 abcdgh     abcdgh
     *       ^            ^
     * 3 abcdgh     abcdgh
     *       ^          ^
     *
     */
    /* 0 */
    int dirty;
    struct cmd *cmd = TERM_CMDLINE(term);

    /* 1 */
    dirty = cmd_delete(cmd, term->col, count);
    if (dirty == -1) {
        return;
    }

    /* 2 */
    DEBUG("dirty: %d", dirty);
    term_rewrite(term, dirty);

    /* 3 */
    term_cursor_move(term, dirty - term->col);
}


void
term_backspace(struct term *term, int count) {
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
    int dirty;
    struct cmd *cmd = TERM_CMDLINE(term);

    if (!term->col) {
        return;
    }

    /* 1 */
    dirty = cmd_delete(cmd, term->col, -count);
    if (dirty == -1) {
        return;
    }

    /* 2 */
    term_rewrite(term, dirty);

    /* 3 */
    term_cursor_move(term, dirty - term->col);
}


int
term_history_rotate(struct term *term, int steps) {
    int r;

    r = term->rotation + steps;
    if ((r < 0) || (r > TERM_HISTORY_COUNT(term))) {
        return -1;
    }

    cmd_commit(TERM_CMDLINE(term));
    term->rotation += steps;
    term_rewrite(term, 0);
    return 0;
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
        /* insert mode */
        while (TERM_INBUFF_COUNT(term)) {
            c = TERM_INBUFF_GET(term);
            //DEBUG("c: %d", c);

#ifdef CONFIG_USH_VI
            /* vi */
            if (vi_mode(&term->vi) == VI_NORMAL) {
                /* normal mode */
                VI_AWAIT(self, viA, &term->vi);
                continue;
            }
#endif  // CONFIG_USH_VI

            /* escape pressed */
            if (c == ASCII_ESC) {
                /* try ansi escape */
                TERM_AWAIT(self, ansiA, term);
#ifdef CONFIG_USH_VI
                if (UAIO_HASERROR(self)) {
                    /* ESC is not eaten by ansiA, switch to normal mode */
                    vi_swmode(&term->vi, VI_NORMAL);
                    TERM_INBUFF_SKIP(term, 1);
                }
#endif  // CONFIG_USH_VI

                continue;
            }

            /* no need to more examination, delete the char from buffer */
            TERM_INBUFF_SKIP(term, 1);

            /* backspace */
            if (ASCII_ISBACKSPACE(c)) {
                term_backspace(term, 1);
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

            /* insert char */
            if (term_insert(term, c)) {
                UAIO_THROW2(self, ENOBUFS);
            }
        }
        fsync(term->outfd);
        EUART_AREAD(self, &term->reader, 1);
    }

    UAIO_FINALLY(self);
}
