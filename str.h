#ifndef STR_H_
#define STR_H_


#include <stddef.h>
#include <stdint.h>


typedef struct str {
    size_t size;
    size_t len;
    char *start;
} str_t;


#undef ERING_PREFIX
#define ERING_PREFIX str
#include <ering.h>


#define STR_AVAIL(s) ((s)->size - (s)->len)
#define STR_FULL(s) (STR_AVAIL(s) == 0)


int
str_init(struct str *s, size_t size);


void
str_deinit(struct str *s);


int
str_append(struct str *s, char c);


int
str_delete(struct str *s, int index);


int
str_copy(struct str *dst, struct str *src);


#endif  // STR_H_
