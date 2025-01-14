#if RPI == 1
#ifndef GPIOLIB_H
#define GPIOLIB_H

extern enum GPIOLIB_E{
    GPIO_READ_E,
} GPIOLIB_E;

extern enum GPIOLIB_P{
    GPIO_SET_MODE_P,
    GPIO_WRITE_P,
    GPIO_READ_P,
    GPIO_SET_PULL_UP_DOWN_P,
    GPIO_DELAY_P,
    GPIO_TERMINATE_P,
} GPIOLIB_P;

#endif
#endif