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
    int maxcol = TERM_CMDLINE(term)->len;
    int steps;

    if (term->vi_mode == VI_NORMAL) {
        maxcol = MAX(0, maxcol - 1);
    }

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

    _printf(term, "%c[%d%c", ASCII_ESC, abs(steps), steps < 0? 'D': 'C');
    term->col = newcol;
}



#define vi_inserting(t) ((t)->vi_mode == VI_INSERT)

static void
_vi_switch(struct term *term, enum vi_mode mode) {
    if (term->vi_mode == mode) {
        return;
    }
    DEBUG("VI: Switched to %s mode, col: %d",
            mode == VI_INSERT? "insert": "normal", term->col);
    term->vi_mode = mode;
    if ((mode == VI_NORMAL) && term->col) {
        _cursor_move(term, -1);
    }
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
_rewrite(struct term *term, int index) {
    struct cmd *cmd = TERM_CMDLINE(term);

    index = MAX(0, index);
    index = MIN(cmd->len - 1, index);

    if (term->col != index) {
        _cursor_move(term, index - term->col);
    }

    DEBUG("index; %d, cmdlen: %d, %.*s", index, cmd->len,
            cmd->len, cmd_ptr(cmd));
    _printf(term, "%s%.*s", ANSI_ERASETOEND, (cmd->len - index),
            cmd_ptroff(cmd, index));
    term->col = cmd->len;
}


static void
_delete(struct term *term, unsigned int count) {
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
    _rewrite(term, dirty);

    /* 3 */
    _cursor_move(term, dirty - term->col);
}


static void
_backspace(struct term *term, int count) {
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
    _rewrite(term, dirty);

    /* 3 */
    _cursor_move(term, dirty - term->col);
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
    _rewrite(term, 0);
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
    _vi_switch(term, VI_INSERT);
    term->vi_repeat = -1;

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
    struct cmd *cmdline = TERM_CMDLINE(term);
    UAIO_BEGIN(self);

aread:
    EUART_AREADT(self, &term->reader, 3, CONFIG_USH_TERM_READER_TIMEOUT_US);
    if (TERM_INBUFF_COUNT(term) < 1) {
        UAIO_RETURN(self);
    }

    c = TERM_INBUFF_GET(term);
    if (c == ASCII_LF) {
        _vi_switch(term, VI_INSERT);
        UAIO_RETURN(self);
    }

    TERM_INBUFF_SKIP(term, 1);

    if (term->vi_repeat == -1) {
        if (ASCII_IS1TO9(c)) {
            term->vi_repeat = c - '0';
            goto aread;
        }
    }
    else if (ASCII_ISDIGIT(c)) {
        term->vi_repeat *= 10;
        term->vi_repeat += c - '0';
        goto aread;
    }

    if (term->vi_repeat <= 0) {
        term->vi_repeat = 1;
    }

    // DEBUG("VI: repeat: %d", term->vi_repeat);

    if (c == 'd') {
        if (TERM_INBUFF_COUNT(term) < 1) {
            EUART_AREAD(self, &term->reader, 1);
        }
        c = TERM_INBUFF_GET(term);
        if (c == '$') {
            _delete(term, cmdline->len);
        }
        else if (c == '0') {
            _delete(term, -cmdline->len);
        }
        else if (c == 'd') {
            _cursor_move(term, -term->col);
            _delete(term, cmdline->len);
        }
        UAIO_RETURN(self);
    }

    switch (c) {
        case 'i':
            _vi_switch(term, VI_INSERT);
            break;

        case 'k':
            if (!_history_rotate(term, term->vi_repeat)) {
                _cursor_move(term, -term->col);
            }
            break;

        case 'j':
            if (!_history_rotate(term, -term->vi_repeat)) {
                _cursor_move(term, -term->col);
            }
            break;

        case 'l':
            _cursor_move(term, term->vi_repeat);
            break;

        case 'h':
            _cursor_move(term, -term->vi_repeat);
            break;

        case '0':
            _cursor_move(term, -term->col);
            break;

        case '$':
            _cursor_move(term, cmdline->len);
            break;

        case 'a':
            _vi_switch(term, VI_INSERT);
            _cursor_move(term, 1);
            break;

        case 'A':
            _vi_switch(term, VI_INSERT);
            _cursor_move(term, cmdline->len);
            break;

        case 'x':
            _delete(term, term->vi_repeat);
            break;

        default:
            WARN("vi command: %c is not supported", c);
            break;
    }

    UAIO_FINALLY(self);
    term->vi_repeat = -1;
}


static ASYNC
_escape(struct uaio_task *self, struct term *term) {
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

        _delete(term, 1);
        skip += 2;
        goto eat;
    }

    switch (c) {
        case 'A':
            _history_rotate(term, 1);
            skip++;
            break;
        case 'B':
            _history_rotate(term, -1);
            skip++;
            break;
        case 'C':
            _cursor_move(term, 1);
            skip++;
            break;
        case 'D':
            _cursor_move(term, -1);
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

            /* vi */
            if (term->vi_mode == VI_NORMAL) {
                /* normal mode */
                TERM_AWAIT(self, _viA, term);
                continue;
            }

            /* escape pressed */
            if (c == ASCII_ESC) {
                /* try ansi escape */
                TERM_AWAIT(self, _escape, term);
                if (UAIO_HASERROR(self)) {
                    /* ESC is not eaten by _escape, switch to normal mode */
                    _vi_switch(term, VI_NORMAL);
                    TERM_INBUFF_SKIP(term, 1);
                }

                continue;
            }

            /* no need to more examination, delete the char from buffer */
            TERM_INBUFF_SKIP(term, 1);

            /* backspace */
            if (ASCII_ISBACKSPACE(c)) {
                _backspace(term, 1);
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
            if (_insert(term, c)) {
                UAIO_THROW2(self, ENOBUFS);
            }
        }
        fsync(term->outfd);
        EUART_AREAD(self, &term->reader, 1);
    }

    UAIO_FINALLY(self);
}
