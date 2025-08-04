#ifndef __OMOTE_LOG_H__
#define __OMOTE_LOG_H__

#include <applicationInternal/hardware/arduinoLayer.h>

#ifdef __cplusplus
extern "C"
{
#endif

#define OMOTE_LOG_LEVEL_NONE       (0)
#define OMOTE_LOG_LEVEL_ERROR      (1)
#define OMOTE_LOG_LEVEL_WARN       (2)
#define OMOTE_LOG_LEVEL_INFO       (3)
#define OMOTE_LOG_LEVEL_DEBUG      (4)
#define OMOTE_LOG_LEVEL_VERBOSE    (5)

#ifndef CONFIG_OMOTE_LOG_DEFAULT_LEVEL
#define CONFIG_OMOTE_LOG_DEFAULT_LEVEL OMOTE_LOG_LEVEL_NONE
#endif

#ifndef OMOTE_LOG_LEVEL
#define OMOTE_LOG_LEVEL CONFIG_OMOTE_LOG_DEFAULT_LEVEL
#endif

#define OMOTE_LOG_FORMAT(letter, format) "[OMOTE " #letter "][%8lu]: " format NEWLINE , (unsigned long) millis()

#if OMOTE_LOG_LEVEL >= OMOTE_LOG_LEVEL_VERBOSE
#define omote_log_v(format, ...) OmoteSerial.printf(OMOTE_LOG_FORMAT(V, format), ##__VA_ARGS__)
#else
#define omote_log_v(format, ...)  do {} while(0)
#endif

#if OMOTE_LOG_LEVEL >= OMOTE_LOG_LEVEL_DEBUG
#define omote_log_d(format, ...) OmoteSerial.printf(OMOTE_LOG_FORMAT(D, format), ##__VA_ARGS__)
#else
#define omote_log_d(format, ...)  do {} while(0)
#endif

#if OMOTE_LOG_LEVEL >= OMOTE_LOG_LEVEL_INFO
#define omote_log_i(format, ...) OmoteSerial.printf(OMOTE_LOG_FORMAT(I, format), ##__VA_ARGS__)
#else
#define omote_log_i(format, ...) do {} while(0)
#endif

#if OMOTE_LOG_LEVEL >= OMOTE_LOG_LEVEL_WARN
#define omote_log_w(format, ...) OmoteSerial.printf(OMOTE_LOG_FORMAT(W, format), ##__VA_ARGS__)
#else
#define omote_log_w(format, ...) do {} while(0)
#endif

#if OMOTE_LOG_LEVEL >= OMOTE_LOG_LEVEL_ERROR
#define omote_log_e(format, ...) OmoteSerial.printf(OMOTE_LOG_FORMAT(E, format), ##__VA_ARGS__)
#else
#define omote_log_e(format, ...) do {} while(0)
#endif

#if OMOTE_LOG_LEVEL >= OMOTE_LOG_LEVEL_NONE
#define omote_log_n(format, ...) OmoteSerial.printf(OMOTE_LOG_FORMAT(E, format), ##__VA_ARGS__)
#else
#define omote_log_n(format, ...) do {} while(0)
#endif

#ifdef __cplusplus
}
#endif

#endif /* __OMOTE_LOG_H__ */
