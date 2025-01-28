#ifndef SETTING_H
#define SETTING_H
//TEST:1 instを表示する

#define TEST 0
//SBC:1 SBCモード // 0:Arduinoモード
#define SBC 1
//DEBUG:1 デバッグモード
#define DEBUG 1



#if SBC
    #define SHICA_PRINTF(...) printf(__VA_ARGS__)
    #define SHICA_FPRINTF(S,...) fprintf(S,__VA_ARGS__)
#else
    #include <Arduino.h>
    #define SHICA_PRINTF(...)   Serial.printf(__VA_ARGS__)
    #define SHICA_FPRINTF(S,...) Serial.printf(__VA_ARGS__)
#endif


//Device
#define RPI 1// 1:RaspberryPi 0:Other


#endif