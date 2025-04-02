
#include "em_timer.h"
#include "em_config.h"


bool read_state_timeout = false;
bool printer_timeout = false;

osTimerId myStateTimerHandle;
osTimerId myTimeoutTimerHandle;

/* read_state_timer_callbackfun function */
void read_state_timer_callbackfun(void const * argument)
{
  /* USER CODE BEGIN read_state_timer_callbackfun */
  printf("read_state now...\r\n");
  read_state_timeout = true;
  /* USER CODE END read_state_timer_callbackfun */
}

void init_timer(){
	osTimerDef(myStateTimer, read_state_timer_callbackfun);
  myStateTimerHandle = osTimerCreate(osTimer(myStateTimer), osTimerPeriodic, NULL);
	osTimerStart(myStateTimerHandle,10000);
}

bool get_state_timeout(){
    return read_state_timeout;
}

void clean_state_timeout(){
    read_state_timeout = false;
}


/* read_state_timer_callbackfun function */
void read_timeout_timer_callbackfun(void const * argument)
{
  /* USER CODE BEGIN read_state_timer_callbackfun */
  printf("¥•∑¢¥Ú”°≥¨ ±¥ÌŒÛ...\r\n");
  printer_timeout = true;
  /* USER CODE END read_state_timer_callbackfun */
}

bool get_printer_timeout_status(){
  return printer_timeout;
}

void open_printer_timeout_timer(){
  printer_timeout = false;
	osTimerDef(myTimeoutTimer, read_timeout_timer_callbackfun);
  myTimeoutTimerHandle = osTimerCreate(osTimer(myTimeoutTimer), osTimerOnce, NULL);
	osTimerStart(myTimeoutTimerHandle,20000);    
}

void close_printer_timeout_timer(){
	osTimerDelete(myTimeoutTimerHandle);
}

