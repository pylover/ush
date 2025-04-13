#ifndef CMD_H_
#define CMD_H_


#include <stddef.h>
#include <stdint.h>

#include "helpers.h"


typedef struct cmd {
    size_t size;
    size_t len;
    char *buff;
    char *back;
} cmd_t;


#define cmd_avail(c) ((c)->size - (c)->len)
#define cmd_isfull(c) (cmd_avail(c) == 0)
#define cmd_isempty(c) ((c)->len == 0)
#define cmd_clear(c) (c)->len = 0
#define cmd_compare(c1, c2) \
    strncmp((c1)->buff, (c2)->buff, MAX((c1)->len, (c2)->len))
#define cmd_ptr(c) (c)->buff
#define cmd_ptroff(c, off) ((c)->buff + (off))
#define cmd_getc(c, idx) ((c)->buff[idx])


int
cmd_init(struct cmd *s, size_t size);


void
cmd_deinit(struct cmd *s);


int
cmd_append(struct cmd *s, char c);


int
cmd_insert(struct cmd *c, char ch, int index);


/** Deletes one or more characters.
 *
 * if index is less than zero, then the index is enumerated from the end of the
 * command, and if count is less than zero, then characters will be deleted
 * backward.
 *
 * returns the dirty portion index.
 * */
int
cmd_delete(struct cmd *s, int index, int count);


int
cmd_copy(struct cmd *dst, struct cmd *src);


void
cmd_commit(struct cmd *c);


void
cmd_restore(struct cmd *c);


#endif  // CMD_H_
