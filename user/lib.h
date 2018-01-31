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
void print(char* s);
uint64_t exit(int64_t ret);
uint64_t getpid();
void printf(char* fmt, ...);
uint64_t fork();