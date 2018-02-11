#pragma once
#include<lib/util.h>
uint64_t strlen(char* p);
int64_t strncmp(char* a, char* b, uint64_t size);
void strncopy(char* t, char* f, uint64_t size);
void memset(char* p, char c, uint64_t size);
int64_t memcmp(char* a, char* b, uint64_t size);
void memcopy(char* t, char* f, uint64_t size);