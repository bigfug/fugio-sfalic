#include "exitit.h"
#include <stdio.h>
#include <stdlib.h>

void exitit(const char * const message, const int code)  
{
	fflush(stdout);
	fflush(stderr);
	if (code>0)
	{
		fputs("\n" EXIT_ERROR_MESSAGE, stderr);
		fputs(message, stderr);
		fflush(stderr);
	}
	else
	{
		fputs("\n", stdout);
		fputs(message, stdout);
		fflush(stdout);
	}
	exit(code);
}

