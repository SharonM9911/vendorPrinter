/**
  ******************************************************************************
  * @file    debug_log.c
  * @brief   SEGGER RTT日志系统实现
  ******************************************************************************
  */
 #include "debug_log.h"
 #include "SEGGER_RTT.h"
 #include "FreeRTOS.h"
 #include "task.h"
 
 /* 私有全局变量 */
 static uint8_t s_log_level = LOG_LEVEL_INFO; // 当前日志级别
 static StaticSemaphore_t xMutexBuffer;
 static SemaphoreHandle_t xLogMutex = NULL;
 
 /* 颜色代码表 */
 static const char* s_log_colors[] = {
     "\033[36m", // DEBUG - 青色
     "\033[32m", // INFO - 绿色
     "\033[33m", // WARN - 黄色
     "\033[31m", // ERROR - 红色
 };
 
 /**
   * @brief 初始化日志系统
   * @note 必须在所有任务启动前调用
   */
 void debug_init(void) {
     // 初始化RTT控制块（关键！）
     SEGGER_RTT_Init();
     
     // 创建互斥锁（FreeRTOS静态分配）
     xLogMutex = xSemaphoreCreateMutexStatic(&xMutexBuffer);
     configASSERT(xLogMutex);
     
     // 配置上行缓冲区（通道0）
     SEGGER_RTT_ConfigUpBuffer(0, "Logger", NULL, 0, 
                              SEGGER_RTT_MODE_NO_BLOCK_TRIM);
     
     // 打印启动标志
     SEGGER_RTT_WriteString(0, "\n\033[35m[RTT] Logger Ready\033[0m\n");
 }
 
 /**
   * @brief 设置日志级别
   * @param level 新日志级别（LOG_LEVEL_XXX）
   */
 void debug_set_level(uint8_t level) {
     if(level <= LOG_LEVEL_ERROR) {
         s_log_level = level;
     }
 }
 
 /**
   * @brief 核心日志输出函数（线程安全）
   * @param level 日志级别
   * @param fmt 格式化字符串
   * @param ... 可变参数
   */
 void debug_log(uint8_t level, const char* fmt, ...) {
     // 级别过滤
     if(level < s_log_level) return;
     
     va_list args;
     char buffer[128];
     int len;
     
     // 添加颜色和级别前缀
     len = snprintf(buffer, sizeof(buffer), "%s[%s] ", 
                   s_log_colors[level], 
                   _get_level_str(level));
     
     // 格式化消息
     va_start(args, fmt);
     len += vsnprintf(buffer + len, sizeof(buffer) - len, fmt, args);
     va_end(args);
     
     // 添加颜色复位和换行
     strcat(buffer, "\033[0m\n");
     
     // 互斥锁保护（带超时）
     if(xSemaphoreTake(xLogMutex, pdMS_TO_TICKS(100)) == pdTRUE) {
         SEGGER_RTT_WriteString(0, buffer);
         xSemaphoreGive(xLogMutex);
     }
 }
 
 /**
   * @brief 中断安全日志（无阻塞）
   * @param msg 固定消息字符串
   */
 void debug_isr_log(const char* msg) {
     SEGGER_RTT_WriteNoLock(0, msg, strlen(msg));
 }
 
 /*********************** 私有函数 ************************/
 /**
   * @brief 获取日志级别字符串
   * @param level 日志级别
   * @retval 级别描述字符串
   */
 static const char* _get_level_str(uint8_t level) {
     static const char* levels[] = {"DBG", "INF", "WRN", "ERR"};
     return (level <= LOG_LEVEL_ERROR) ? levels[level] : "???";
 }