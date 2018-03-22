#include "prism/log.h"

#include <stdio.h>
#include <stdarg.h>

void logFormatFunc(char* tFormatString, ...) {
	char text[1024];
	va_list args;
	va_start(args, tFormatString);
	vsprintf(text, tFormatString, args);
	va_end(args);

	printf("%s\n", text);
}

void logErrorFormatFunc(char * tFormatString, ...)
{
	char text[1024];
	va_list args;
	va_start(args, tFormatString);
	vsprintf(text, tFormatString, args);
	va_end(args);

	printf("%s\n", text);
}
