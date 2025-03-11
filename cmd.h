#ifndef CMD_H_
#define CMD_H_


#include <stddef.h>
#include <stdint.h>

#include "helpers.h"


typedef struct cmd {
    size_t size;
    size_t len;
    char *buff;
} cmd_t;


#define cmd_avail(s) ((s)->size - (s)->len)
#define cmd_isfull(s) (cmd_avail(s) == 0)
#define cmd_isempty(s) ((s)->len == 0)
#define cmd_clear(s) (s)->len = 0
#define cmd_compare(a, b) \
    strncmp((a)->buff, (b)->buff, MAX((a)->len, (b)->len))


int
cmd_init(struct cmd *s, size_t size);


void
cmd_deinit(struct cmd *s);


int
cmd_append(struct cmd *s, char c);


int
cmd_delete(struct cmd *s, int index);


int
cmd_copy(struct cmd *dst, struct cmd *src);


#endif  // CMD_H_
