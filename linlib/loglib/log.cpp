#include "log.h"

pthread_mutex_t 		m_lock;
bool 					m_is_first = true;
std::string 			m_log_filename;
std::string 			m_base_log_filename = "./log";
int 					m_log_size = 10 * 1024 *1024;
FILE*       			m_fp = NULL; 
char        			m_time_str[MAX_TIME_STR_LEN];
int 					m_write_len;
int 					m_log_level = LOG_LEVEL_WARNING;

void init();
void rotate_log();
void rename_file();
char* get_time_str(bool is_write);
void lock();
void unlock();

void 	set_log_filepath(const char * filepath)
{
	m_base_log_filename = filepath;
}
void 	set_log_level(int level)
{
	m_log_level = (level > LOG_LEVEL_MAX) ? LOG_LEVEL_MAX : level;
}
void 	set_log_size(int size)
{
	m_log_size = size;
}
int 	writeline(int level, const char* format_str, ...)
{
	if(m_is_first == true){
		init();
		m_is_first = false;
	}
	
	if (m_log_level < level) return 0;
	
	
	if (m_fp == NULL){
		return -1;
	}
	lock();
	if (ftell(m_fp) >= m_log_size)
    {
        rotate_log();//切换日志写入
    }
	m_write_len = fprintf(m_fp, "[%s][%s]", get_time_str(true), s_level_str[level-1]);
	va_list p_list;
    va_start(p_list, format_str);
    m_write_len += vfprintf(m_fp, format_str, p_list);
    va_end(p_list);
    m_write_len += fprintf(m_fp, "\n");
    fflush(m_fp);
    unlock();

    return m_write_len;	
}




void init()
{
	pthread_mutex_init(&m_lock, NULL);
	rotate_log();
	
}
void rotate_log()
{
	FCLOSE(m_fp);
	rename_file();
	m_fp = fopen(m_log_filename.c_str(), "wb+");
    if (NULL == m_fp)
    {
        fprintf(stderr, "Open log file(%s) fail!\n", m_log_filename.c_str());
    }
}
void rename_file()
{
    std::string new_name = m_base_log_filename + "-" + get_time_str(false);
    m_log_filename = new_name;
}
char* get_time_str(bool is_write)
{
    time_t now = {0};
    struct tm *ptime = NULL;
    time(&now);
    ptime = localtime(&now);
    memset(m_time_str, 0, sizeof(m_time_str));

    uint32 milisec = 0;
#ifdef WIN32
    SYSTEMTIME wtm;
    GetLocalTime(&wtm);
    milisec = wtm.wMilliseconds;
#else
    struct timeval tv = {0};
    gettimeofday(&tv, 0);
    milisec = tv.tv_usec/1000;
#endif

    if (is_write)
    {//用来写日志
        sprintf(m_time_str, "%04d%02d%02d-%02d:%02d:%02d.%06ld",
            (1900+ptime->tm_year), (1+ptime->tm_mon), ptime->tm_mday,
            ptime->tm_hour, ptime->tm_min, ptime->tm_sec, milisec);
    }
    else
    {//用来重命名文件
        sprintf(m_time_str, "%04d-%02d-%02d-%02d_%02d_%02d_%06ld",
            (1900+ptime->tm_year), (1+ptime->tm_mon), ptime->tm_mday,
            ptime->tm_hour, ptime->tm_min, ptime->tm_sec, milisec);
    }

    return m_time_str;
}

void lock() 
{
	pthread_mutex_lock(&m_lock);
}
void unlock() 
{
	pthread_mutex_unlock(&m_lock);
}


