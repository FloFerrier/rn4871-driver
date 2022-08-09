/**
 * \file    logs.h
 * \defgroup Logs used by RN4871 driver and the virtual module
 * \copyright nothing
 *
 * \brief   No description.
 * \details Nothing yet.
 *
 * @{
 */

#ifndef LOGS_H
#define LOGS_H

#define LOG_BUFFER_LEN_MAX 255

enum log_level_e {
    LOG_ERROR = (1 << 0),
    LOG_DEBUG = (1 << 1),
    LOG_INFO  = (1 << 2),
};

/*!
 * \details 
 *
 * \param[in] level
 * \param[in] message
 *
 */
void logger(enum log_level_e level, const char *message, ...);

extern void rn4871LogSenderAPI(char *log, int logLen);

#endif /* LOGS_H */

/**@}*/