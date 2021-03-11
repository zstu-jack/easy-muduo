//
// Created by jackyan on 2021/3/11.
//
#ifndef LOG_H
#define LOG_H
#include <string>
#include <fstream>
#include <time.h>
#include <sys/types.h>
#include <dirent.h>
#include <stdarg.h>


enum LOG_LEVEL{ // Use bit mask instead.
    NONE = -1,
    DETAIL = 0,
    WARNING = 1,
    ERROR = 2,
    FATAL = 3,
};

static std::string log_level_string[FATAL+1] = {" [detail]", "[warning]", "  [error]", "  [fatal]"};

static std::string log_directory = "./logs/";

static std::string timestamp()
{
    char str[9];
    auto ts = time(0);
    auto* p = localtime(&ts);
    strftime(str, 9, "%H:%M:%S", p);
    return str;
}
static std::string date(){
    char str[11];
    auto ts = time(0);
    auto* p = localtime(&ts);
    strftime(str, 11, "%Y-%m-%d", p);
    return str;
}

class Logger{
public:
    Logger(const Logger& rhs) = delete;
    Logger operator=(const Logger& rhs) = delete;
    Logger(int32_t level, const std::string& filename_template);
    void open_file(const std::string& now_date);
    void log(int32_t level, const std::string& log_str);
    void log(int32_t level, const char* buffer, ...) __attribute__((format(printf, 3, 4)));
    void log(int32_t level, const char* buffer, va_list va);
public:
    int32_t m_log_level;
    std::fstream m_logfile;
    std::string m_logfile_template;
    std::string m_last_date; // change to int. 20200202
};

#endif //LOG_H
