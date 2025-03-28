#ifndef SETTING_H
#define SETTING_H
//TEST:1 instを表示する

#ifndef TEST
#define TEST 0
#endif

#ifndef DEBUG
//DEBUG:1 デバッグモード
#define DEBUG 0
#endif

#ifndef SBC
//SBC:1 SBCモード // 0:Arduinoモード
#define SBC 1
#endif


#ifndef RPI
#define RPI 0// 1:RaspberryPi 0:Other
#endif

#ifndef ROS
//LIB
#define ROS 0// 1:ROS 0:Not use
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

