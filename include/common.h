#ifndef clox_common_h
#define clox_common_h

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define NAN_BOXING
//#define DEBUG_PRINT_CODE
//#define DEBUG_TRACE_EXECUTION
//#define DEBUG_STRESS_GC
//#define DEBUG_LOG_GC

#define ANSI_COLOR_RED     					"\x1b[31m"
#define ANSI_COLOR_GREEN   					"\x1b[32m"
#define ANSI_COLOR_YELLOW  					"\x1b[33m"
#define ANSI_COLOR_BLUE    					"\x1b[34m"
#define ANSI_COLOR_MAGENTA 					"\x1b[35m"
#define ANSI_COLOR_CYAN    					"\x1b[36m"
#define ANSI_COLOR_BRIGHT_YELLOW    "\x1b[32m"
#define ANSI_COLOR_RESET   					"\x1b[0m"

#define COLOR_BOLD  "\e[1m"
#define COLOR_OFF   "\e[m"


#define UINT8_COUNT	(UINT8_MAX	+	1)

#endif
