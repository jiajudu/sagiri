#pragma once
#include "lib/stdint.h"
void mminit();
uint64_t alloc();
void free(uint64_t p);