#include "em_spi.h"

//static const int spiClk = 1000000; // 1 MHz

//SPIClass hspi  = SPIClass(HSPI);
//SPISettings printerSPISettings = SPISettings(1000000, SPI_MSBFIRST, SPI_MODE0);

void spiCommand(uint8_t *data_buffer, uint8_t data_len)
{
//    hspi.beginTransaction(printerSPISettings);
//    hspi.transfer(data_buffer, data_len);
//    hspi.endTransaction();
		HAL_SPI_Transmit(&hspi1, data_buffer, data_len, 0xFFFF);
}

void init_spi()
{
//    hspi.begin(PIN_SCK, -1, PIN_SDA, -1); // SCLK, MISO, MOSI, SS
//    hspi.setFrequency(2000000);
}


