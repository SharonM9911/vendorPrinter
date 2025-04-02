#include "em_hal.h"
#include "em_adc.h"
#include "main.h"

int freq = 2000;    //����Ƶ��2000kHz
int channel = 0;    //ͨ���ţ�ȡֵ0 ~ 15
int resolution = 8; //�ֱ��ʣ�ȡֵ0~20��ռ�ձ�duty���ȡֵΪ2^resolution-1

bool need_report = false;


#define EPISON 1e-7

static int digitalRead(int pin){
	return HAL_GPIO_ReadPin(VHINT_GPIO_Port,VHINT_Pin);
}

static void digitalWrite(int pin,int PinState){
	HAL_GPIO_WritePin(GPIOB,LED_Pin,(GPIO_PinState)PinState);
}

static long map(long x, long in_min, long in_max, long out_min, long out_max) {
    const long dividend = out_max - out_min;
    const long divisor = in_max - in_min;
    const long delta = x - in_min;
    if(divisor == 0){
        return -1; //AVR returns -1, SAM returns 0
    }
    return (delta * dividend + (divisor / 2)) / divisor + out_min;
}

void read_battery()
{
		get_device_state()->battery = map(get_adc_volts()*2,3300,4200,0,100);
	  if(get_device_state()->battery > 100)
      get_device_state()->battery = 100;
    LOG_INFO("battery = %d\n",get_device_state()->battery);
}

void read_temperature()
{
    float temperatrue = get_adc_temperatrue();
    LOG_INFO("temperature = %f\n",temperatrue);
    if(temperatrue >= EPISON){
        get_device_state()->temperature = temperatrue;
    }else{
        get_device_state()->temperature = 0;
    }
}

void run_beep(beep_type_e type){

}

void run_led(led_type_e type){
    switch (type)
    {
    case LED_CONNECT:
        digitalWrite(PIN_LED, 0);
        break;
		case LED_DISCONNECT:
				digitalWrite(PIN_LED, 1);
        break;
    case LED_WARN:
        digitalWrite(PIN_LED, 0);
        vTaskDelay(200);
        digitalWrite(PIN_LED, 1);
        vTaskDelay(200);
        digitalWrite(PIN_LED, 0);
        vTaskDelay(200);
        digitalWrite(PIN_LED, 1);
        vTaskDelay(200);
        digitalWrite(PIN_LED, 0);
        break;
    case LED_PRINTER_START:
				digitalWrite(PIN_LED, 1);
				vTaskDelay(200);
        digitalWrite(PIN_LED, 0);
        vTaskDelay(200);
        break;
		case LED_BLE_INIT:
				digitalWrite(PIN_LED, 0);
        vTaskDelay(10);
        digitalWrite(PIN_LED, 1);
    default:
        break;
    }
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	if(GPIO_Pin & GPIO_PIN_8)
	{
		LOG_INFO("--");
    need_report = true;
    set_device_paper_status(PAPER_STATUS_LACK);
	}
}

void read_paper_statue()
{
    if(digitalRead(PIN_PAPER) == PAPER_STATUS_NORMAL){
        get_device_state()->paper_state = PAPER_STATUS_NORMAL;
    }else{
        get_device_state()->paper_state = PAPER_STATUS_LACK;
    }
    LOG_INFO("paper_state = %d\n",get_device_state()->paper_state);
}

bool read_paper_irq_need_report_status()
{
    if (need_report)
    {
        need_report = false;
        return true;
    }
    else
        return false;
}

void read_all_hal()
{
    read_battery();
    read_temperature();
    read_paper_statue();
}

void init_hal()
{
    //ȱֽIO��ʼ��
    // pinMode(PIN_PAPER, INPUT);
    // attachInterrupt(PIN_PAPER, paper_isr, RISING);
	
	digitalWrite(PIN_LED, 1);
	

}

