#if ROS
#ifndef ROSLIB_H
#define ROSLIB_H

extern enum STDLIB_E{
    ANY_ROS_EVENT_E,
} ROSLIB_E;

extern enum STDLIB_P{
    ANY_ROS_FUNC_P,
    ANY_ROS_FUNC_OF_EVENT_P,
} ROSLIB_P;

#endif //ROSLIB_H
#endif //ROS