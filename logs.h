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
    LOG_ERROR = 0x1,
    LOG_DEBUG = 0x2,
    LOG_INFO  = 0x4,
};

/*!
 * \details This API allows to get the mac address for the module.
 *
 * \param[in,out] dev : Structure instance of rn4871_dev_s.
 * \param[out]    macAddress : Buffer that contains mac address string.
 *
 * \return Result of API execution status
 * \retval CODE_RETURN_ERROR
 * \retval CODE_RETURN_SUCCESS
 */
void logger(enum log_level_e level, const char *message, ...);

/*!
 * \details This API depends on your target, it must be declare on your system.
 * \note Typically, the implementation is the following :
 * \code
 *  void rn4871LogSenderAPI(char *log, int logLen) {
 *      printf("%s", log);
 *  }
 * \endcode
 *
 * \param[in] log : Log buffer.
 * \param[in] logLen : Size of the log buffer.
 *
 * \return Result of API execution status
 * \retval CODE_RETURN_ERROR
 * \retval CODE_RETURN_SUCCESS
 */
extern void rn4871LogSenderAPI(char *log, int logLen);

#endif /* LOGS_H */

/**@}*/