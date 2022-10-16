#ifndef RN4871_LOGGER_H
#define RN4871_LOGGER_H

#ifdef __cplusplus
extern "C" {
#endif

#define LOG_BUFFER_LEN_MAX 255

enum log_level_e
{
    LOG_ERROR = 0x1,
    LOG_DEBUG = 0x2,
    LOG_INFO = 0x4,
};

typedef enum log_level_e RN4871_LOG_LEVEL;

void logger(RN4871_LOG_LEVEL level, const char *message, ...);

extern void rn4871LogSender(char *log, int logLen);

#ifdef __cplusplus
}
#endif

#endif /* RN4871_LOGGER_H */