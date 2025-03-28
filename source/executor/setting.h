#ifndef SETTING_H
#define SETTING_H
//TEST:1 instを表示する

#ifndef TEST
//TEST:1 instを表示する
#define TEST 0
#endif

#ifndef DEBUG
//DEBUG:1 デバッグモード
#define DEBUG 0
#endif

#ifndef MSGC
//MSGC:1 独自GCを使用する
#define MSGC 0
#endif

#ifndef MAXTHREADSIZE
//QUEUE_SIZE:10 キューサイズ event関数の最大Stock数
#define MAXTHREADSIZE 10
#endif

#ifndef SBC
//SBC:1 PC or single board computerの時
#define SBC 1
#endif


#ifndef UNMARK
#define UNMARK 1
#endif

#ifndef RPI
//RPI:1 RaspberryPiの時
#define RPI 0
#endif

#ifndef ROS
//Library
#define ROS 0
#endif

#if SBC
    #define SHICA_PRINTF(...) printf(__VA_ARGS__)
    #define SHICA_FPRINTF(S,...) fprintf(S,__VA_ARGS__)
#else
    #include <Arduino.h>
    #define SHICA_PRINTF(...)   Serial.printf(__VA_ARGS__)
    #define SHICA_FPRINTF(S,...) Serial.printf(__VA_ARGS__)
#endif

#endif //SETTING_H