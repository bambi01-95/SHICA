#ifndef LIBRARY_H
#define LIBRARY_H

extern enum LIB{
    STDLIB,
    USERLIB,
#if RPI
    GPIOLIB,
#endif //RPI
}LIB;

#endif