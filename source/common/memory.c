#if COMPILER_C
    #include "../compiler/setting.h"
#else
    #include "../executor/setting.h" //need to consider the compile time
#endif

#ifndef MEMORY_C
#define MEMORY_C

#if SBC
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


// メモリをファイルに書き込む関数
void memoryWriteC(const char *path) {
    // デバッグ表示
    printf("Path: %s\n", path);

    // ファイルを開く
    FILE *fp = fopen(path, "w");
    if (!fp) {
        perror("Error opening file");
        return;
    }

    // メモリデータの検証
    if (!memory || memsize == 0) {
        fprintf(stderr, "Error: Invalid memory or memsize\n");
        fclose(fp);
        return;
    }

    // ファイルの先頭にヘッダーを書き込む
    fputs("#ifndef MEMORY_C\n", fp);
    fputs("#define MEMORY_C\n", fp);
    fputs("#include <stdint.h>\n", fp);
    fputs("const uint8_t memory[] = {\n", fp);

    // メモリデータを16進数表記で書き込む
    // if (memsize != fwrite(memory, 1, memsize, fp))
    for (size_t i = 0; i < memsize; i++) {
        fprintf(fp, "0x%02X%s", memory[i], (i < memsize - 1) ? ", " : "");
        if ((i + 1) % 8 == 0) fputs("\n", fp);
    }
    // フッターを書き込む
    fputs("", fp);
    fputs("};\n#endif\n", fp);

    // ファイルを閉じる
    if (fclose(fp) != 0) {
        perror("Error closing file");
    }
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
#else
#include <stdio.h>
#include <sys/stat.h>

int has_extension(const char *filename,char *end) {
    printf("has_extension is not support\n");
    return 0;
}

typedef unsigned char byte;
// byte   *memory  = 0;  // memory is a huge array of bytes
// size_t  memsize = 0;  // this is the current size of data stored in memory
// size_t  memcap  = 0;  // this is the maximum dize of data that memory can hold



void _error(char *msg)
{
    printf("\n%s(%d) ");
    exit(1);
}

#define error(X) _error(X, __FILE__, __LINE__)

void memoryWrite(char *path) // write memory to a file
{
    printf("memoryWrite is not support\n");
    exit(1);
}

/* READ */

void memoryRead(char *path) // read memory from an external file
{
    printf("memoryRead is not support\n");
}


void _genByte(byte b)  // append one byte the the memory
{
    printf("_genByte is not support\n");
}

void memoryClear(void) // clear the contents of memory
{
    printf("memoryClear is not support\n");
}
#endif

#endif