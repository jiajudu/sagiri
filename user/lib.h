#pragma once
typedef char int8_t;
typedef short int16_t;
typedef int int32_t;
typedef long int64_t;
typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;
typedef unsigned long uint64_t;
#define nullptr 0
#define false 0
#define true 1
typedef int64_t bool;
struct dircontent{
    char c[14 * 32][16];
};
struct stat{
    uint64_t type;
    uint64_t size;
};
void print(char* s);
uint64_t exit(int64_t ret);
uint64_t getpid();
uint64_t gettid();
void printf(char* fmt, ...);
uint64_t fork();
uint64_t thread(uint64_t fn, uint64_t* args);
uint64_t threadexit(int64_t ret);
uint64_t waitproc(uint64_t pid, int64_t* addr);
uint64_t waitthread(uint64_t tid, int64_t* addr);
uint64_t killproc(uint64_t pid);
uint64_t killthread(uint64_t tid);
uint64_t sleep(uint64_t tick);
uint64_t open(char* name, uint64_t flag);
uint64_t close(uint64_t fd);
uint64_t read(uint64_t fd, char* buf, uint64_t size);
uint64_t write(uint64_t fd, char* buf, uint64_t size);
uint64_t unlink(char* name);
uint64_t readdir(char* name, struct dircontent* buf);
uint64_t stat(char* name, struct stat* buf);
uint64_t mkdir(char* name);
uint64_t put(char s);