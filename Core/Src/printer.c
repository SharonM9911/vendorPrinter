// printer.c
void Execute_ESC_POS(const uint8_t* data, uint16_t len) {
    // 硬件流控检查
    while(Printer_Busy()) {
        HAL_Delay(10);
    }
    
    // DMA发送（非阻塞）
    HAL_UART_Transmit_DMA(&huart1, data, len);
    
    // 记录最后活动时间
    last_print_time = HAL_GetTick();
}