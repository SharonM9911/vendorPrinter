// Core/debug/debug_log.h
#pragma once
#include <stdarg.h>
#include "FreeRTOS.h"
#include "semphr.h"
// 日志级别定义
typedef enum {
    LOG_LEVEL_DEBUG,
    LOG_LEVEL_INFO,
    LOG_LEVEL_WARN,
    LOG_LEVEL_ERROR
} LogLevel_t;
// 日志级别控制（通过Keil的全局宏定义）
#ifdef DEBUG_LEVEL
    #define LOG_LEVEL DEBUG_LEVEL
#else
    #define LOG_LEVEL 2  // 默认INFO级别
#endif
// API声明
void debug_init(void);
void debug_set_level(uint8_t level);
void debug_log(uint8_t level, const char* fmt, ...);
void debug_isr_log(const char* msg);

// 快捷宏
#ifndef DEBUG
#define LOG_DBG(fmt, ...) debug_log(LOG_LEVEL_DEBUG, fmt, ##__VA_ARGS__)
#define LOG_ERR(fmt, ...) debug_log(LOG_LEVEL_ERROR, fmt, ##__VA_ARGS__)
#define LOG_INFO(fmt, ...) debug_log(LOG_LEVEL_INFO, fmt, ##__VA_ARGS__)
#endif

#define LOG(level, ...) \
    do { if(level >= LOG_LEVEL) SEGGER_RTT_printf(0, __VA_ARGS__); } while(0)