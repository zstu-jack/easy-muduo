#include <string>

#include "Log.h"
#include "Util.h"


void Logger::log(int32_t log_level, const std::string& log_str)
{
    if(log_level < this->m_log_level){
        return ;
    }
    if(log_level < 0 || log_level > FATAL){
        log_level = FATAL;
    }

    std::string now_date = date();
    if(now_date != m_last_date){
        m_last_date = now_date;
        open_file(now_date);
    }
    std::string message;
    message = log_level_string[log_level] + message;
    message += "[" + timestamp() + "] ";
    message += log_str;
    m_logfile << message;
    m_logfile.flush();
}
void Logger::log(int32_t level, const char* buffer, ...) {
    va_list ap;
    va_start(ap, buffer);
    std::string log_str = string_format(buffer, ap);
    va_end(ap);
    log(level, log_str);
}
void Logger::log(int32_t level, const char* buffer, va_list va){
    std::string log_str = string_format(buffer, va);
    log(level, log_str);
}


void Logger::open_file(const std::string& now_date){
    std::string filename = std::string("./logs/") + m_logfile_template + now_date;
    std::fstream fs(filename.c_str(), std::ios::in);
    if (fs.is_open()){
        fs.close();
        m_logfile.open(filename.c_str(), std::ios::out | std::ios::app);
    }else{
        m_logfile.open(filename.c_str(), std::ios::out);
    }
}

Logger::Logger(int32_t level, const std::string& filename_template){
    DIR* dir = opendir("./logs");
    if (dir){
        closedir(dir);         // Directory exists.
    } else{
        std::string cmd = std::string("mkdir -p ") + std::string("./logs/");
        system(cmd.c_str());
    }
    m_logfile_template = filename_template;
    std::string now_date = date();
    open_file(now_date);
    m_last_date = now_date;
    m_log_level = level;
}
