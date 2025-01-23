#ifndef LIBRARY_H
#define LIBRARY_H

extern enum LIB{
    STDLIB,
    COMMUNICATELIB,
    USERLIB,
#if RPI
    GPIOLIB,
#endif //RPI
}LIB;

#endif