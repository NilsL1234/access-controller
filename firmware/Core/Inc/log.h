#ifndef LOG_H
#define LOG_H


typedef enum{
    LOG_INFO = 0x00,
    LOG_WARN = 0x01,
    LOG_ERROR = 0x02
} LOG_Level;

void LOG_Write(LOG_Level Level, const char *Source, const char *Message);




#endif