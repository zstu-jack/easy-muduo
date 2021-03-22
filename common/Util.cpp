//
// Created by jackyan on 2021/3/11.
//
#include <memory>

#include "Util.h"


std::string string_format(const char* fmt_str, va_list vl) {

    const int small_buffer_size = 1024;
    char small_buffer[small_buffer_size];
//    va_start(vl, fmt_str);
    int ret = vsnprintf(small_buffer, small_buffer_size, fmt_str, vl);
//    va_end(vl);
    if(ret >= 0 && ret < small_buffer_size){
        return std::string(small_buffer);
    }

    int final_n, n = ret + 1;
    if(ret < 0){
        n = 4096;
    }

    // fixme: another va_list.
    std::unique_ptr<char[]> formatted;
    for(int i = 0; i < 3; i ++){
        formatted.reset(new char[n]); /* Wrap the plain char array into the unique_ptr */
        final_n = vsnprintf(&formatted[0], n, fmt_str, vl);
        if (final_n < 0 || final_n >= n)
            n *= 2;
        else
            break;
    }
    return std::string(formatted.get());
}
