#pragma once
#include<lib/util.h>
void consoleput(int64_t c);
void consolewritec(char c);
int64_t consoleopen(uint64_t flags);
int64_t consoleclose(uint64_t fdn);
int64_t consoleread(uint64_t fdn, char* buf, uint64_t size);
int64_t consolewrite(uint64_t fdn, char* buf, uint64_t size);