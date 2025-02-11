#ifndef EXTSTR_C
#define EXTSTR_C
#include <assert.h>

/*
if it is prim or struct type, it is 0
if it is pointer of prim or struct type, it is 1

e.g.,
struct Socket{
    int fd;        // 0
    int port;      // 0
    char *ip;      // 1
};

int ret = registerExternType(0b100); // register Socket type
assert(ret == 0);

*/


typedef unsigned int extstr;
extstr ExternStructMap[32];
int     numExternStruct = 0;

extstr registerExternType(extstr pointerMap){
    if(numExternStruct < sizeof(ExternStructMap)/sizeof(extstr))
        return 0; //error
    for(int i = 0; i < numExternStruct; i++){
        if(ExternStructMap[i] == pointerMap)
            return i;
    }
    ExternStructMap[numExternStruct] = pointerMap;
    return numExternStruct++;
}

#endif