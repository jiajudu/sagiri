#pragma once
#include<lib/util.h>
extern uint64_t freeblocks;
extern uint64_t freememory;
uint64_t alloc();
void free(uint64_t p);
uint64_t malloc();
void mfree(uint64_t p);
void malloctest();