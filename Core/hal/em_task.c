#include "em_task.h"
#include "em_button.h"
#include "em_motor.h"
#include "em_printer.h"
#include "em_adc.h"

bool printer_test = false;
bool is_long_click_start = false;

void key_click_handle()
{
    LOG_INFO("Button ����!\n");
    printer_test = true;
    // read_all_hal();
}

void key_long_click_handle()
{
		if(is_long_click_start == true)
			return;
		is_long_click_start = true;
    LOG_INFO("Button ����!\n");
    device_state_t *pdevice = get_device_state();
    bool need_beep = false;
    // ��ȱֽ�Ҳ��ڴ�ӡ�в�ִ��
    if (pdevice->paper_state == PAPER_STATUS_NORMAL)
    {
        if (pdevice->printer_state == PRINTER_STATUS_FINISH ||
            pdevice->printer_state == PRINTER_STATUS_INIT)
        {
            LOG_INFO("��ʼ��ֽ\n");
            motor_start();
        }
        else
        {
            need_beep = true;
        }
    }
    else
        need_beep = true;
    if (need_beep)
    {
        run_beep(BEEP_WARN);
				run_led(LED_WARN);
    }
}

void key_long_click_free_handle()
{
		is_long_click_start = false;
    LOG_INFO("ֹͣ��ֽ\n");
    motor_stop();
}


/**
 * @brief �����ϱ�����¼�
 *
 */
void run_report()
{
    if (get_state_timeout())
    {
				clean_state_timeout();
        read_all_hal();
        if (get_ble_connect())
        {
						LOG_INFO("report device status:report time up\n");
            ble_report();
        }
    }
    if (read_paper_irq_need_report_status())
    {
        // ȱֽ�жϲ�������Ҫ�ϱ�
        LOG_INFO("report device status : paper irq\n");
        ble_report();
    }
}

/**
 * @brief ������ӡ����¼�
 *
 */
void run_printer()
{
    device_state_t *pdevice = get_device_state();
		#ifdef START_PRINTER_WHEN_FINISH_RAED
        if (pdevice->read_ble_finish == true)
        {
            if (pdevice->printer_state == PRINTER_STATUS_FINISH ||
                pdevice->printer_state == PRINTER_STATUS_INIT)
            {
                pdevice->read_ble_finish = false;
                pdevice->printer_state = PRINTER_STATUS_START;
                ble_report();
                LOG_INFO("report device status : printing start %d\n",get_ble_rx_leftline());
                run_beep(BEEP_PRINTER_START);
                run_led(LED_PRINTER_START);
            }
        }
    #else
			// ���մ���100��ʱ���Ŵ�����ʼ��ӡ
			if (get_ble_rx_leftline()> 200)
			{
					if (pdevice->printer_state == PRINTER_STATUS_FINISH ||
							pdevice->printer_state == PRINTER_STATUS_INIT)
					{
							pdevice->printer_state = PRINTER_STATUS_START;
							ble_report();
							LOG_INFO("report device status : printing start\n");
							run_beep(BEEP_PRINTER_START);
							run_led(LED_PRINTER_START);
					}
			}
		#endif
    // ��ʼ��ӡ
    if (pdevice->printer_state == PRINTER_STATUS_START)
    {
        // ������ӡ
        start_printing_by_queuebuf();
        pdevice->printer_state = PRINTER_STATUS_FINISH;
    }
}

void task_report(void *pvParameters)
{
		int count = 0;
		LOG_INFO("task_report init\n");
    for (;;) // A Task shall never return or exit.
    {
        run_report();
        vTaskDelay(100);
				count ++;
				if(count >= 50){
					count = 0;
					LOG_INFO("task_report run\n");
				}
    }
}

void task_button(void *pvParameters)
{
		int count = 0;
		LOG_INFO("task_button init\n");
    for (;;) // A Task shall never return or exit.
    {
        key_check_run(); // ��Ҫ���ڵ��ð�����������
        vTaskDelay(20);
				count ++;
				if(count >= 250){
					count = 0;
					LOG_INFO("task_button run\n");
				}
    }
}

void task_printer(void *pvParameters)
{
		int count = 0;
		init_ble();
		LOG_INFO("task_printer init\n");
    for (;;) // A Task shall never return or exit.
    {
				ble_status_data_clean();
        run_printer();
        vTaskDelay(1);
        if(printer_test){
            printer_test = false;
            testSTB();
        }
				count ++;
				if(count >= 5000){
					count = 0;
					LOG_INFO("task_printer run\n");
				}
    }
}

//void printer_run(){
//    // printer_test = true;
//    for (;;) // A Task shall never return or exit.
//    {
//        run_printer();
//        // vTaskDelay(10);
//        if(printer_test){
//            printer_test = false;
//            testSTB();
//        }
//    }
//}

void init_task()
{
    LOG_INFO("init_task\n");
    init_device_state();
    init_timer();
    init_hal();
    init_queue();
    adc_init();
    init_key();
    init_printer();
    xTaskCreate(
        task_report,  // ������
        "TaskReport", // ������
        128,         // ����ջ
        NULL,         // �������
        1,            // �������ȼ�, with 3 (configMAX_PRIORITIES - 1) ����ߵģ�0����͵�.
        NULL          // ������
    );

    xTaskCreate(
        task_button,  // ������
        "TaskButton", // ������
        128,         // ����ջ
        NULL,         // �������
        0,            // �������ȼ�, with 3 (configMAX_PRIORITIES - 1) ����ߵģ�0����͵�.
        NULL          // ������
    );

    xTaskCreate(
        task_printer,  // ������
        "TaskPrinter", // ������
        256,          // ����ջ
        NULL,          // �������
        2,             // �������ȼ�, with 3 (configMAX_PRIORITIES - 1) ����ߵģ�0����͵�.
        NULL           // ������
    );
}


