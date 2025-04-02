#ifndef _EM_TASK_H_
#define _EM_TASK_H_

#include "em_config.h"
#include "em_device.h"
#include "em_timer.h"
#include "em_ble.h"
#include "em_hal.h"
#include "em_queue.h"

void init_task(void);

void printer_run(void);

void key_click_handle(void);

void key_long_click_handle(void);

void key_long_click_free_handle(void);


#endif

