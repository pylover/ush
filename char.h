#ifndef CHAR_H_
#define CHAR_H_


#define CHAR_LF 10
#define CHAR_ESCAPE 27
#define CHAR_BACKSPACE 127


#define ISNAV(c) (((c) == CHAR_ESCAPE) || ((c) == CHAR_BACKSPACE))


#endif  // CHAR_H_
