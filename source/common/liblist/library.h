#ifndef LIBRARY_H
#define LIBRARY_H

extern enum LIB{
    STDLIB,
    COMMUNICATELIB,
    USERLIB,
#if RPI
    GPIOLIB,
#endif //RPI
#if ROS
    ROSLIB,
#endif //ROS
}LIB;

#endif