#ifndef __Log_h__
#define __Log_h__


#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string>


#include <stdarg.h>         //vsnprintf,va_start,va_end
#include <unistd.h>         //access
#include <sys/stat.h>       //mkdir
#include <sys/time.h>       //gettimeofday
#include <dirent.h>         //opendir,readdir,closedir
#include <string.h> 

#define MAX_TIME_STR_LEN    30
#define FCLOSE(fp)    {if (NULL != fp) {fclose(fp); fp = NULL;}}
typedef unsigned long uint32;
static const char* s_level_str[] = {"ERROR", "WARNING", "INFO", "TRACE"};



//日志级别
enum _log_level
{
    LOG_LEVEL_ERROR     = 1,  //错误
    LOG_LEVEL_WARNING   = 2,  //警告
    LOG_LEVEL_INFO      = 3,  //普通
    LOG_LEVEL_TRACE     = 4,  //调试
    LOG_LEVEL_MAX
};


void 	set_log_filepath(const char * filepath);
void 	set_log_level(int level);
void 	set_log_size(int size);
int 	writeline(int level, const char* format_str, ...);

#define SET_LOG_NAME(filepath)              set_log_filepath(filepath)
#define SET_LOG_LEVEL(level)                set_log_level(level)
#define SET_LOG_SIZE(size)                  set_log_size(size)

#define LOG_TRACE(format, ...)             writeline(LOG_LEVEL_TRACE, format, ## __VA_ARGS__)
#define LOG_INFO(format, ...)              writeline(LOG_LEVEL_INFO, format, ## __VA_ARGS__)
#define LOG_WARNING(format, ...)           writeline(LOG_LEVEL_WARNING, format, ## __VA_ARGS__)
#define LOG_ERROR(format, ...)             writeline(LOG_LEVEL_ERROR, format, ## __VA_ARGS__)





#endif // __Log_h__

