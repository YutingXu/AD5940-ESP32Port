#ifndef ESP32PORT_H
#define ESP32PORT_H

#include "ad5940.h"

#include "esp_log.h"

#include "driver/spi_master.h"
#include "driver/gpio.h"
#include "rom/ets_sys.h"

#ifdef CONFIG_IDF_TARGET_ESP32
#define SENDER_HOST HSPI_HOST

#else
#define SENDER_HOST SPI2_HOST

#endif

// change the GPIO pins as per the particular configuration you are using
// this configuration works for metro esp32-s2 boards
// this routes the SPI signals through the GPIO MUX, hence will be slower, the clock will only be able to run up to 40MHz
// #define GPIO_SCLK      42   // D13 in Arduino UNO terms
// #define GPIO_MISO      21   // D12 in Arduino UNO terms
// #define GPIO_MOSI      16   // D11 in Arduino UNO terms
// #define GPIO_CS        15   // D10 in Arduino UNO terms
// #define AD5940_RST_PIN 2    // A3/D17 in Arduino UNO terms
// #define AD5940_GP0INT_PIN 14 // A1 in Arduino UNO terms

// this pin configuration is for devkitc-v4
#define GPIO_SCLK      18   // D13 in Arduino UNO terms
#define GPIO_MISO      19   // D12 in Arduino UNO terms
#define GPIO_MOSI      23   // D11 in Arduino UNO terms
#define GPIO_CS        5    // D10 in Arduino UNO terms, this is a dummy pin that will not be used (AD5940 library was programmed in such a way that chip selects are handled manually)
#define AD5940_CS_PIN  16    // this is the true CS pin, AD5940 will not work with the default CS pin.
#define AD5940_GP0INT_PIN 14 // A1 in Arduino UNO terms
#define AD5940_RST_PIN 17    // A3/D17 in Arduino UNO terms

spi_device_handle_t spi_handle; // globle handle to be in line with porting requirements

volatile static uint8_t ucInterrupted = 0;       /* Flag to indicate interrupt occurred */

/**
 * @brief Pull !CS pin high
*/
void AD5940_CsSet(void)
{
   gpio_set_level(AD5940_CS_PIN, 1); // not sure if this will work
}

/**
 * @brief Pull !CS pin low
*/
void AD5940_CsClr(void)
{
   gpio_set_level(AD5940_CS_PIN, 0); // not sure if this will work
}

/**
 * @brief Pull !RESET pin high
*/
void AD5940_RstSet(void)
{
   gpio_set_level(AD5940_RST_PIN, 1); // assuming that initialisation has been done
}

/**
 * @brief Pull !RESET pin low
*/
void AD5940_RstClr(void)
{
   gpio_set_level(AD5940_RST_PIN, 0); // assuming that initialisation has been done
}

uint32_t AD5940_GetMCUIntFlag(void)
{
   return ucInterrupted;
}

uint32_t AD5940_ClrMCUIntFlag(void)
{
   ucInterrupted = 0;
   return 1;
}

/**
 * @brief Block the processor for a certain amount of time. Total delay is 10*time microseconds
 * @param time: number of 10us delays.
 * @return None
*/
void AD5940_Delay10us(uint32_t time)
{
    if(time == 0)
        return;

    ets_delay_us(time * 10);
}

/**
  @brief Using SPI to transmit one byte and return the received byte. 
  @param pSendBuffer: Pointer to the data to be sent
    - Set to NULL to skip write phase
  @param pRecvBuff: Pointer to the buffer used to store received data.
    - Set to NULL to skip read phase
  @param length: data length in SendBuffer in bytes
  @note this function does not use command bits for compatibility with the AD5940 library
  @return None
**/
void AD5940_ReadWriteNBytes(unsigned char *pSendBuffer,unsigned char *pRecvBuff,unsigned long length)
{
    spi_transaction_t t;
    memset(&t, 0, sizeof(t));

    t.tx_buffer = pSendBuffer;
    t.rx_buffer = pRecvBuff;
    t.length = length*8;

    spi_device_acquire_bus(spi_handle, portMAX_DELAY);
    spi_device_transmit(spi_handle, &t);
    spi_device_release_bus(spi_handle);
}

/**
  @brief Initialise SPI and GPIO peripherals for ESP32. 
  @param pCfg: Optional configuration flags.
  @return always 0.
**/
uint32_t AD5940_MCUResourceInit(void *pCfg)
{
    printf("Attempting to initialise MCU...\n");
	// Step1, initalise SPI perpheral and GPIO
	// Configuration for the SPI bus
	spi_bus_config_t buscfg={
		.mosi_io_num = GPIO_MOSI,
		.miso_io_num = GPIO_MISO,
		.sclk_io_num = GPIO_SCLK,
		.quadwp_io_num = -1,
		.quadhd_io_num = -1
	};

	// Configuration for the SPI device on the other side of the bus
    spi_device_interface_config_t devcfg={
        .command_bits = 0,
        .address_bits = 0,
        .dummy_bits = 0,
        .clock_speed_hz = SPI_MASTER_FREQ_8M,
        .duty_cycle_pos = 128,        // 50% duty cycle
        .mode = 0,
        .spics_io_num = GPIO_CS,
        .cs_ena_posttrans = 0,        // does not matter, not using the SPI CS pin anyways
        .queue_size = 1
    };

	// GPIO config for the reset pin.
    gpio_config_t adf5940_rst_conf = {
        .intr_type = GPIO_INTR_DISABLE,
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = 1,
        .pin_bit_mask = (1 << AD5940_RST_PIN)
    };

    gpio_config_t ad5940_int_conf = {
        .intr_type = GPIO_INTR_POSEDGE,
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = 1,
        .pin_bit_mask = (1 << AD5940_GP0INT_PIN)
    };

    gpio_config_t ad5940_cs_conf = {
        .intr_type = GPIO_INTR_DISABLE,
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = 1,
        .pin_bit_mask = (1 << AD5940_CS_PIN)
    };

	gpio_config(&adf5940_rst_conf);
    gpio_config(&ad5940_int_conf);
    gpio_config(&ad5940_cs_conf);

    gpio_set_level(AD5940_CS_PIN, 1); // pull CS high, there were scenarios where this was pulled low despite it being defined as pull-up

    printf("GPIO successfully configured");

	esp_err_t ret;

	ret = spi_bus_initialize(SENDER_HOST, &buscfg, SPI_DMA_CH_AUTO);
    assert(ret == ESP_OK);

	ret = spi_bus_add_device(SENDER_HOST, &devcfg, &spi_handle);
	assert(ret == ESP_OK);

    printf("SPI device successfully attached");

    return 0;
}

#endif
