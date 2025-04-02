#ifndef _EM_TIMER_H_
#define _EM_TIMER_H_

#include "em_config.h"

/**
 * @brief 初始化Timer
 * 
 */
void init_timer(void);

/**
 * @brief Get the state timeout objectd
 * 
 * @return true 
 * @return false 
 */
bool get_state_timeout(void);

/**
 * @brief 
 * 
 */
void clean_state_timeout(void);

/**
 * @brief 开启打印超时计时器
 * 
 */
void open_printer_timeout_timer(void);

/**
 * @brief 关闭打印超时计时器
 * 
 */
void close_printer_timeout_timer(void);

/**
 * @brief Get the printer timeout status object
 * 
 * @return true 
 * @return false 
 */
bool get_printer_timeout_status(void);

#endif
