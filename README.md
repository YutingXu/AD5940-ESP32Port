# AD5940-ESP32Port
Local port for ESP32 for the AD5940 library

This port was created by following the instructions found [here](https://wiki.analog.com/resources/eval/user-guides/eval-ad5940/tools/porting_source_code).

## How to use
To use the port, put ESP32Port.c in the same directory as the the AD5940 library.

For example, if you were using Platformio with VSCode as your IDE, your directory should look someth9ing like this:

```
project
└───lib
│   │
│   └───AD5940
│       │   ad5940.c
│       │   ad5940.h
│       │   ESP32Port.c
│   
└───src
    │   main.c
    │   ...
```

## Results
This library was tested on ESP32-devkitC-v4 board connected to a AD5940ELCZ. However, it should theoretically work on all other ESP32 boards. Only the SPI test was conducted at 8MHz

## Disclaimer
This library is in no way endorsed by ADI and usage is at your own risk.
