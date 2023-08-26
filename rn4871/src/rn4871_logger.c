#include "rn4871_logger.h"

#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

char logBuffer[LOG_BUFFER_LEN_MAX + 1] = "";

void logger(RN4871_LOG_LEVEL level, const char *message, ...) {
  assert(NULL != message);

#ifdef RN4871_LOG_ENABLE
  if (!(RN4871_LOG_ENABLE & level)) {
    return;
  }

  va_list args;
  va_start(args, message);

  int logBufferLen = 0;

  switch (level) {
    case LOG_ERROR:
      logBufferLen = snprintf(logBuffer, LOG_BUFFER_LEN_MAX, "[ERROR] ");
      break;
    case LOG_DEBUG:
      logBufferLen = snprintf(logBuffer, LOG_BUFFER_LEN_MAX, "[DEBUG] ");
      break;
    case LOG_INFO:
      logBufferLen = snprintf(logBuffer, LOG_BUFFER_LEN_MAX, "[INFO] ");
      break;
    default:
      break;
  }

  vsnprintf(&logBuffer[logBufferLen], LOG_BUFFER_LEN_MAX - logBufferLen,
            message, args);

  logBufferLen = (int)strlen(logBuffer);
  rn4871LogSender(logBuffer, logBufferLen);

  va_end(args);
#else
  return;
#endif
}