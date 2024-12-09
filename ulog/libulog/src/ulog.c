#include "ulog.h"
#include <stdarg.h>
#include <stdio.h>
#include <time.h>

void ulog_set_global_log_level(int level) {
    global_log_level = level;
}

int ulog_get_global_log_level() {
    return global_log_level;
}

// ulog function
void ulog_log(int level, const char *fmt, ...) 
{
    if (level > global_log_level) 
	{
        return; 
    }

    time_t now = time(NULL);
    struct tm *tm_info = localtime(&now);
    char time_buffer[26];
    strftime(time_buffer, 26, "%Y-%m-%d %H:%M:%S", tm_info);

    va_list args;
    va_start(args, fmt);
    printf("[%s]", time_buffer);
    vprintf(fmt, args);
    printf("\n");
    va_end(args);
}