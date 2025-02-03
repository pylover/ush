#ifndef STR_H_
#define STR_H_


#include <stddef.h>
#include <stdint.h>


typedef struct str {
    size_t len;
    char *start;
} str_t;


#undef ERING_PREFIX
#define ERING_PREFIX str
#include <ering.h>


// struct str *
// str_new(size_t size);
//
//
// void
// str_dispose(struct str *s);


#endif  // STR_H_
