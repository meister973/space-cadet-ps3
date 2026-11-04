#include <stdarg.h>
#include <stdio.h>

#include <sysutil/sysutil.h>

#include "system.h"

static volatile bool exit_requested;
static FILE *log_file;

static void on_system_event(u64 status, u64 param, void *user_data)
{
    (void)param;
    (void)user_data;
    if (status == SYSUTIL_EXIT_GAME)
        exit_requested = true;
}

void system_open(void)
{
    log_file = fopen(SYSTEM_LOG_PATH, "w");
    sysUtilRegisterCallback(SYSUTIL_EVENT_SLOT0, on_system_event, NULL);
}

bool system_exit_requested(void)
{
    sysUtilCheckCallback();
    return exit_requested;
}

void system_log(const char *format, ...)
{
    va_list arguments;

    va_start(arguments, format);
    vprintf(format, arguments);
    va_end(arguments);
    fflush(stdout);

    if (log_file == NULL)
        return;
    va_start(arguments, format);
    vfprintf(log_file, format, arguments);
    va_end(arguments);
    fflush(log_file);
}
