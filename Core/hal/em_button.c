#include "em_button.h"
#include "em_task.h"

//�̰�ʱ��Ϊ < 1s
#define SHORT_PRESS_TIME 1000

bool keyIsPress = false;
unsigned long clicktime = 0;
bool longPressIsStart = false;

static int digitalRead(int pin){
	return HAL_GPIO_ReadPin(BTN_GPIO_Port,BTN_Pin);
}

static uint32_t millis(){
	return HAL_GetTick();
}

void key_check_run(){
  if(keyIsPress == false){
    if(digitalRead(PIN_KEY) == LOW){
      vTaskDelay(10);
      if(digitalRead(PIN_KEY) == LOW){
        keyIsPress = true;
        clicktime = millis();
      }
    }
  }
  if(keyIsPress == true){
    if(digitalRead(PIN_KEY) == HIGH){
      if(millis() - clicktime > SHORT_PRESS_TIME){
        //LOG_INFO("long click free\n");
        key_long_click_free_handle();
        longPressIsStart = false;
      }else{
        //LOG_INFO("short click\n");
        key_click_handle();
      }
      keyIsPress = false;
    }else{
      if(millis() - clicktime > SHORT_PRESS_TIME){
        if(longPressIsStart == false){
          //LOG_INFO("long click\n");
          key_long_click_handle();
          longPressIsStart = true;
        }
      }
    }
  }
}

void init_key()
{
    //�Ѿ���stm32cubemx���ó�ʼ��
}

