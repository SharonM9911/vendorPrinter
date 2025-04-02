#include "em_printer.h"
#include "em_timer.h"

float addTime[6] = {0};
// ����-����ʱ��ϵ��
#define kAddTime 0.001
// ���ݴ�ӡͷʵ�ʴ�ӡЧ���޸Ĵ�ӡʱ��ƫ��ֵ
#define STB1_ADDTIME 0
#define STB2_ADDTIME 0
#define STB3_ADDTIME 0
#define STB4_ADDTIME 0
#define STB5_ADDTIME 0
#define STB6_ADDTIME 0
// ���ܶ�
uint8_t heat_density = 64;


static void digitalWrite(int pin,int PinState){
	if(pin == PIN_STB1){
		HAL_GPIO_WritePin(GPIOB,STB1_Pin,(GPIO_PinState)PinState);
	}else if(pin == PIN_STB2){
		HAL_GPIO_WritePin(GPIOB,STB2_Pin,(GPIO_PinState)PinState);
	}else if(pin == PIN_STB3){
		HAL_GPIO_WritePin(GPIOB,STB3_Pin,(GPIO_PinState)PinState);
	}else if(pin == PIN_STB4){
		HAL_GPIO_WritePin(GPIOB,STB4_Pin,(GPIO_PinState)PinState);
	}else if(pin == PIN_STB5){
		HAL_GPIO_WritePin(GPIOB,STB5_Pin,(GPIO_PinState)PinState);
	}else if(pin == PIN_STB6){
		HAL_GPIO_WritePin(GPIOB,STB6_Pin,(GPIO_PinState)PinState);
	}else if(pin == PIN_LAT){
		HAL_GPIO_WritePin(GPIOB,LAT_Pin,(GPIO_PinState)PinState);
	}
}

static void digitalWrite_vhen(int pin,int PinState){
	HAL_GPIO_WritePin(GPIOA,VH_EN_Pin,(GPIO_PinState)PinState);
}

/**
 * @brief ʧ������ͨ��
 *
 */
static void set_stb_idle()
{
    digitalWrite(PIN_STB1, LOW);
    digitalWrite(PIN_STB2, LOW);
    digitalWrite(PIN_STB3, LOW);
    digitalWrite(PIN_STB4, LOW);
    digitalWrite(PIN_STB5, LOW);
    digitalWrite(PIN_STB6, LOW);
}

/**
 * @brief ��ӡǰ��ʼ��
 *
 */
static void init_printing()
{
    // ������ӡ��ʱ����
    open_printer_timeout_timer();
    set_stb_idle();
    digitalWrite(PIN_LAT, HIGH);
    // POWER ON
    digitalWrite_vhen(PIN_VHEN, HIGH);
}

/**
 * @brief ��ӡ��ֹͣ
 *
 */
static void stop_printing()
{
    // �رմ�ӡ��ʱ����
    close_printer_timeout_timer();
    // POWER OFF
    digitalWrite_vhen(PIN_VHEN, LOW);
    set_stb_idle();
    digitalWrite(PIN_LAT, HIGH);
}

/**
 * @brief Set the heat density object�ܶ�����
 *
 * @param density
 */
void set_heat_density(uint8_t density)
{
    LOG_INFO("��ӡ�ܶ����� %d\n", density);
    heat_density = density;
}

void clearAddTime()
{
    addTime[0] = addTime[1] = addTime[2] = addTime[3] = addTime[4] = addTime[5] = 0;
}

/**
 * @brief ����һ������
 *
 * @param data
 */
static void send_one_line_data(uint8_t *data)
{
    float tmpAddTime = 0;
    clearAddTime();
    for (uint8_t i = 0; i < 6; ++i)
    {
        for (uint8_t j = 0; j < 8; ++j)
        {
            addTime[i] += data[i * 8 + j];
        }
        tmpAddTime = addTime[i] * addTime[i];
        addTime[i] = kAddTime * tmpAddTime;
    }
    spiCommand(data, TPH_DI_LEN);
    /* After send one dot line, send LAT signal low pulse.*/
    digitalWrite(PIN_LAT, LOW);
    us_delay(LAT_TIME);
    digitalWrite(PIN_LAT, HIGH);
}

/**
 * @brief ͨ����ӡ����
 *
 * @param now_stb_num
 */
static void run_stb(uint8_t now_stb_num)
{
    switch (now_stb_num)
    {
    case 0:
        digitalWrite(PIN_STB1, 1);
        us_delay((PRINT_TIME + addTime[0] + STB1_ADDTIME) * ((double)heat_density / 100));
        digitalWrite(PIN_STB1, 0);
        us_delay(PRINT_END_TIME);
        break;
    case 1:
        digitalWrite(PIN_STB2, 1);
        us_delay((PRINT_TIME + addTime[1] + STB2_ADDTIME) * ((double)heat_density / 100));
        digitalWrite(PIN_STB2, 0);
        us_delay(PRINT_END_TIME);
        break;
    case 2:
        digitalWrite(PIN_STB3, 1);
        us_delay((PRINT_TIME + addTime[2] + STB3_ADDTIME) * ((double)heat_density / 100));
        digitalWrite(PIN_STB3, 0);
        us_delay(PRINT_END_TIME);
        break;
    case 3:
        digitalWrite(PIN_STB4, 1);
        us_delay((PRINT_TIME + addTime[3] + STB4_ADDTIME) * ((double)heat_density / 100));
        digitalWrite(PIN_STB4, 0);
        us_delay(PRINT_END_TIME);
        break;
    case 4:
        digitalWrite(PIN_STB5, 1);
        us_delay((PRINT_TIME + addTime[4] + STB5_ADDTIME) * ((double)heat_density / 100));
        digitalWrite(PIN_STB5, 0);
        us_delay(PRINT_END_TIME);
        break;
    case 5:
        digitalWrite(PIN_STB6, 1);
        us_delay((PRINT_TIME + addTime[5] + STB6_ADDTIME) * ((double)heat_density / 100));
        digitalWrite(PIN_STB6, 0);
        us_delay(PRINT_END_TIME);
        break;
    default:
        break;
    }
}

/**
 * @brief �ƶ����&��ʼ��ӡ
 *
 * @param need_stop
 * @param stbnum
 */
bool move_and_start_std(bool need_stop, uint8_t stbnum)
{
    if (need_stop == true)
    {
				LOG_INFO("��ӡֹͣ\n");
        motor_stop();
        stop_printing();
        return true;
    }
    // 4stepһ��
    motor_run();
    if (stbnum == ALL_STB_NUM)
    {
        // ����ͨ����ӡ
        for (uint8_t index = 0; index < 6; index++)
        {
            run_stb(index);
            // �ѵ�������źŲ���ͨ�������У����ٴ�ӡ���ٺͺ�ʱ
            if (index == 1 || index == 3 || index == 5)
            {
                motor_run();
            }
        }
				// motor_run_step(3);
    }
    else
    {
        // ��ͨ����ӡ
        run_stb(stbnum);
        motor_run_step(3);
    }
    return false;
}

/**
 * @brief ��ӡ������
 * 
 * @param need_report �Ƿ���BLE�ϱ�
 * @return true ��ӡ����
 * @return false ��ӡ����
 */
bool printing_error_check(bool need_report)
{
    if (get_printer_timeout_status())
    {
				LOG_INFO("��ӡ��ʱ\n");
        return true;
    }
    if (get_device_state()->paper_state == PAPER_STATUS_LACK)
    {
        if(need_report){
            // ֹͣ��ӡ
            clean_printbuffer();
            ble_report();
        }
        // ֹͣ��ӡ
        LOG_INFO("ȱֽ\n");
        run_beep(BEEP_WARN);
				run_led(LED_WARN);
        return true;
    }
    if (get_device_state()->temperature > 65)
    {
        if(need_report){
            // ֹͣ��ӡ
            clean_printbuffer();
            ble_report();
        }
        // ֹͣ��ӡ
        LOG_INFO("�¶��쳣\n");
        run_beep(BEEP_WARN);
				run_led(LED_WARN);
        return true;
    }
    return false;
}


/**
 * @brief �����ӡ
 *
 * @param data
 * @param length ���ݳ��ȱ��������� 48*n
 */
void start_printing(uint8_t *data, uint32_t len)
{
    uint32_t offset = 0;
    uint8_t *ptr = data;
    bool need_stop = false;
    init_printing();
    while (1)
    {
        if (len > offset)
        {
            // ����һ������ 48byte*8=384bit
            send_one_line_data(ptr);
            offset += TPH_DI_LEN;
            ptr += TPH_DI_LEN;
        }
        else
            need_stop = true;
        if (move_and_start_std(need_stop, ALL_STB_NUM))
            break;
        if(printing_error_check(false))
            break;
    }
    motor_run_step(40);
    motor_stop();
    LOG_INFO("��ӡ���\n");
}

/**
 * @brief �ɱ���д�ӡ
 *
 */
void start_printing_by_queuebuf()
{
    uint8_t *pdata = NULL;
		uint32_t printer_count = 0;
    init_printing();
    while (1)
    {
        if (get_ble_rx_leftline() > 0)
        {
            // LOG_INFO("printing...\n");
            pdata = read_to_printer();
            if (pdata != NULL)
            {
								printer_count ++;
                send_one_line_data(pdata);
                if (move_and_start_std(false, ALL_STB_NUM))
                    break;
            }
        }
        else
        {
            if (move_and_start_std(true, ALL_STB_NUM))
                break;
        }
        if (get_printer_timeout_status())
            break;
        if(printing_error_check(true))
            break;
    }
    motor_run_step(140);
    motor_stop();
    clean_blepack_count();
    LOG_INFO("printer finish !!! read=%d printer:%d\n",get_blepack_count(),printer_count);
}

/**
 * @brief ��ͨ�������ӡ
 *
 * @param stbnum
 * @param data
 * @param len
 */
void start_printing_by_onestb(uint8_t stbnum, uint8_t *data, uint32_t len)
{
    uint32_t offset = 0;
    uint8_t *ptr = data;
    bool need_stop = false;
    init_printing();
    while (1)
    {
        LOG_INFO("printer %d\n", offset);
        if (len > offset)
        {
            // ����һ������ 48byte*8=384bit
            send_one_line_data(ptr);
            offset += TPH_DI_LEN;
            ptr += TPH_DI_LEN;
        }
        else
            need_stop = true;
        if (move_and_start_std(need_stop, stbnum))
            break;
        if (get_printer_timeout_status())
            break;
        if(printing_error_check(false))
            break;
    }
		LOG_INFO("printer end\n");
    motor_run_step(40);
    motor_stop();
		LOG_INFO("printer end2\n");
}

static void setDebugData(uint8_t *print_data)
{
    for (uint32_t cleardata = 0; cleardata < 48 * 5; ++cleardata)
    {
        print_data[cleardata] = 0x55;
    }
}

void testSTB()
{
    uint8_t print_data[48 * 6];
    uint32_t print_len;
    LOG_INFO("��ʼ��ӡ��ӡͷѡͨ���Ų���\n˳��: 1  2  3  4  5  6");
    print_len = 48 * 5;
    setDebugData(print_data);
    start_printing_by_onestb(0, print_data, print_len);
    setDebugData(print_data);
    start_printing_by_onestb(1, print_data, print_len);
    setDebugData(print_data);
    start_printing_by_onestb(2, print_data, print_len);
    setDebugData(print_data);
    start_printing_by_onestb(3, print_data, print_len);
    setDebugData(print_data);
    start_printing_by_onestb(4, print_data, print_len);
    setDebugData(print_data);
    start_printing_by_onestb(5, print_data, print_len);
    LOG_INFO("�������");
}

void init_printer()
{
    init_motor();

//    pinMode(PIN_LAT, OUTPUT);
//    pinMode(PIN_SCK, OUTPUT);
//    pinMode(PIN_SDA, OUTPUT);

//    pinMode(PIN_STB1, OUTPUT);
//    pinMode(PIN_STB2, OUTPUT);
//    pinMode(PIN_STB3, OUTPUT);
//    pinMode(PIN_STB4, OUTPUT);
//    pinMode(PIN_STB5, OUTPUT);
//    pinMode(PIN_STB6, OUTPUT);
    set_stb_idle();

//    pinMode(PIN_VHEN, OUTPUT);
    digitalWrite_vhen(PIN_VHEN, LOW);

    init_spi();
}
