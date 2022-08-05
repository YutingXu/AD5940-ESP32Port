# AD5940-ESP32Port
Porting AD5940 library to ESP32. This port allows ESP32 to interface with the low-power AFE AD5940 created by Analog Devices.

This port was created by following the instructions found [here](https://wiki.analog.com/resources/eval/user-guides/eval-ad5940/tools/porting_source_code).

## How to use
To use the port, put ESP32Port.c in the same directory as the the AD5940 library.

For example, if you were using Platformio with VSCode as your IDE, your directory should look something like this:

```
project
└───lib
│   │
│   └───AD5940
│       │   ad5940.c
│       │   ad5940.h
│       │   --> ESP32Port.c
│   
└───src
    │   main.c
    │   ...
```

Also, you should add a 10us delay between the SPI transactions in the function ```AD5940_SPIWriteReg()``` just to be safe with the timing:
```
static void AD5940_SPIWriteReg(uint16_t RegAddr, uint32_t RegData)
{  
  /* Set register address */
  AD5940_CsClr();
  AD5940_ReadWrite8B(SPICMD_SETADDR);
  AD5940_ReadWrite16B(RegAddr);
  AD5940_CsSet();
  /* Add delay here to meet the SPI timing. */
  AD5940_Delay10us(1);
  AD5940_CsClr();
  AD5940_ReadWrite8B(SPICMD_WRITEREG);
  if(((RegAddr>=0x1000)&&(RegAddr<=0x3014)))
    AD5940_ReadWrite32B(RegData);
  else
    AD5940_ReadWrite16B(RegData);
  AD5940_CsSet();
}
```

## Results
This library was tested on ESP32-devkitC-v4 board connected to a AD5940ELCZ. The SPI test in test.c passed with a master clock of 8MHz. More tests will be done with the library and this section will be updated accordingly. 

## Using boards other than ESP32-devkitC-v4
The library should theoretically work on all other ESP32 boards providing that you changed the pin definitions for SPI, reset pin and the interrupt pin. Note that due to the way the AD5940 library was programmed, a dummy CS pin was added. The true CS pin is defined as ```AD5940_CS_PIN``` and will be manually controlled.

## Disclaimer
This library is in no way endorsed by ADI and usage is at your own risk. Feel free to point out any mistakes in the code.
