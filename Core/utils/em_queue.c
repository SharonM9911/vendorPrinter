#include "em_queue.h"
#include "semphr.h"

ble_rx_t g_ble_rx;

SemaphoreHandle_t xHandler = NULL;

void init_queue()
{
		clean_printbuffer();
    xHandler = xSemaphoreCreateMutex();
}

/**
 * @brief 写入一行数据
 *
 * @param pdata
 * @param length
 */
void write_to_printbuffer(uint8_t *pdata, size_t length)
{
		static BaseType_t xHigherPriorityTaskWoken;
    if (length == 0)
        return;
    if (g_ble_rx.left_line >= MAX_LINE)
        return;
    if (length > MAX_ONELINE_BYTE)
        length = MAX_ONELINE_BYTE;
    // 查看是否可以获得信号量，如果信号量不可用，则用10个时钟滴答来查看信号量是否可用
    if (xSemaphoreTakeFromISR(xHandler, &xHigherPriorityTaskWoken) == pdPASS)
    {
        memcpy(&g_ble_rx.printer_buffer[g_ble_rx.w_index], pdata, length);
        g_ble_rx.w_index++;
        g_ble_rx.left_line++;
        if (g_ble_rx.w_index >= MAX_LINE)
            g_ble_rx.w_index = 0;
        if (g_ble_rx.left_line >= MAX_LINE)
            g_ble_rx.left_line = MAX_LINE;
        xSemaphoreGiveFromISR(xHandler,&xHigherPriorityTaskWoken);
    }
		if(xHigherPriorityTaskWoken == pdTRUE){
			// 如果有更高优先级的任务需要唤醒，则进行任务切换
			portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
		}
}

/**
 * @brief 读取一行数据
 *
 * @return uint8_t*
 */
uint8_t *read_to_printer()
{
    uint32_t index = 0;
    // 查看是否可以获得信号量，如果信号量不可用，则用10个时钟滴答来查看信号量是否可用
    if (xSemaphoreTake(xHandler, (portTickType)10) == pdPASS)
    {
        if (g_ble_rx.left_line > 0)
        {
            g_ble_rx.left_line--;
            index = g_ble_rx.r_index;
            g_ble_rx.r_index++;
            if (g_ble_rx.r_index >= MAX_LINE)
                g_ble_rx.r_index = 0;
            xSemaphoreGive(xHandler);
            return g_ble_rx.printer_buffer[index].buffer;
        }
        xSemaphoreGive(xHandler);
        return NULL;
    }else
        return NULL;
}

/**
 * @brief 清空接收缓存
 *
 */
void clean_printbuffer()
{
    g_ble_rx.w_index = 0;
    g_ble_rx.r_index = 0;
    g_ble_rx.left_line = 0;
}

uint32_t get_ble_rx_leftline()
{
    return g_ble_rx.left_line;
}
