#ifndef SETTING_H
#define SETTING_H
//TEST:1 instを表示する

#define TEST 0
//MSGC:1 独自GCを使用する
#define MSGC 0
//QUEUE_SIZE:10 キューサイズ event関数の最大Stock数
#define MAXTHREADSIZE 10
//SBC:1 PC or single board computerの時
#define SBC 1
//DEBUG:1 デバッグモード
#define DEBUG 0

#if SBC
    #define SHICA_PRINTF(...) printf(__VA_ARGS__)
    #define SHICA_FPRINTF(S,...) fprintf(S,__VA_ARGS__)
#else
    #include <Arduino.h>
    #define SHICA_PRINTF(...)   Serial.printf(__VA_ARGS__)
    #define SHICA_FPRINTF(S,...) Serial.printf(__VA_ARGS__)
#endif

#define UNMARK 1

//Device
#define RPI 0 // 1:RaspberryPi 0:Other

#endif