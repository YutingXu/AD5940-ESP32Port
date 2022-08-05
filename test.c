#include "ad5940.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>

#include "string.h"

// freeRTOS related includes
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// driver related includes
#include "driver/uart.h"
#include "driver/spi_master.h"
#include "driver/gpio.h"
#include "esp_timer.h"


void app_main() 
{
  AD5940_MCUResourceInit(NULL);

	printf("MCU Initialised\n");

	// reset AD5940
	printf("Attempting to reset AD5940...\n");
	AD5940_HWReset();
	printf("AD5940 reset!\n");

	// initialise AD5940 by writing the startup sequence
	printf("Attempting to initialise AD5940...\n");
	AD5940_Initialize();
	printf("AD5940 initialised!\n");

	unsigned long temp;

  temp = AD5940_ReadReg(REG_AFECON_CHIPID);
  printf("Read ADIID register, got: 0x%04lx\n", temp);
  if(temp != 0x5502)
      printf("Read register test failed.\n" );
  else
      printf("Read register test passed\n");

	// write register test
  srand(0x1234);
  int i = 10000;
	while(i--)
	{
		static unsigned long count;
		static unsigned long data;
		/* Generate a 32bit random data */
		data = rand()&0xffff;
		data <<= 16;
		data |= rand()&0xffff;
		count ++;	/* Read write count */
		/**
		 * Register CALDATLOCK is 32-bit width, it's readable and writable.
		 * We use it to test SPI register access.
		*/
		AD5940_WriteReg(REG_AFE_CALDATLOCK, data);
		temp = AD5940_ReadReg(REG_AFE_CALDATLOCK);
		if(temp != data)
		    printf("Write register test failed @0x%08lx\n", data);
		if(!(count%1000))
		    printf("Read/Write has been done %ld times, latest data is 0x%08lx\n", count, data);
	}
  printf("SPI read/write test completed");
}

