#ifndef CONFIG_H_
#define CONFIG_H_


#include "sdkconfig.h"


#ifdef CONFIG_USH_LINEBREAK_LF
#define LINEBREAK "\n"
#elif CONFIG_USH_LINEBREAK_CRLF
#define LINEBREAK "\r\n"
#endif


#endif  // CONFIG_H_
