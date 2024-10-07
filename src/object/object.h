#ifndef OBJECT_H
#define OBJECT_H
union Object;
typedef union Object Object;
typedef Object *oop;
typedef oop (*Func)(oop);
#endif