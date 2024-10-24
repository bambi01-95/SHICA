#ifndef MEMORY_C
#define MEMORY_C
#include <stdio.h>
#include <sys/stat.h>

int has_extension(const char *filename,char *end) {
    const char *ext = strrchr(filename, '.');  // ファイル名中の最後の'.'を探す
    if (ext != NULL && strcmp(ext, end) == 0) {
        return 1;  // .txt拡張子がある場合は1を返す
    }
    return 0;      // それ以外の場合は0を返す
}

typedef unsigned char byte;
byte   *memory  = 0;  // memory is a huge array of bytes
size_t  memsize = 0;  // this is the current size of data stored in memory
size_t  memcap  = 0;  // this is the maximum dize of data that memory can hold

const unsigned int SIZE_INST   = sizeof(unsigned char);
const unsigned int SIZE_INT    = sizeof(int);            //size of int
const unsigned int SIZE_LONG   = sizeof(long long int);  //size of long long int
const unsigned int SIZE_FLOAT  = sizeof(float);          //size of float
const unsigned int SIZE_DOUBLE = sizeof(double);         //size of double

void _error(char *msg, char *file, int line)
{
    fprintf(stderr, "\n%s(%d) ", file, line);
    perror(msg);
    exit(1);
}

#define error(X) _error(X, __FILE__, __LINE__)

void memoryWrite(char *path) // write memory to a file
{
    FILE *fp = fopen(path, "w");
    if (!fp) error(path);
    if (memsize != fwrite(memory, 1, memsize, fp)) // write memory to file
	error(path);
    fclose(fp);
}

/* READ */

void memoryRead(char *path) // read memory from an external file
{
    struct stat buf;
    if (stat(path, &buf)) error(path); // get file information including size

    FILE *fp = fopen(path, "r");
    if (!fp) error(path);

    memsize = memcap = buf.st_size; // set memory size same as file size
    memory = malloc(memsize);       // allocate just the right amount of memory
    if (!memory) error("malloc");

    if (memsize != fread(memory, 1, memsize, fp)) // read memory from file
	error(path);
    fclose(fp);
}


void _genByte(byte b)  // append one byte the the memory
{
    if  (memsize >= memcap) { // memory is full, extend it
        memcap = memcap ? memcap * 2 : 1024;   // 1k, 2k, 4k, 8k, 16k, ...
        memory = realloc(memory, memcap);
    }
    assert(memsize < memcap);
    memory[memsize++] = b;
}

void memoryClear(void) // clear the contents of memory
{
    free(memory);
    memory  = 0;
    memsize = 0;
    memcap  = 0;
}

#endif