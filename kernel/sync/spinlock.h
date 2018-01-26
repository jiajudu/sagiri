#pragma once
#include<lib/util.h>
struct spinlock{
    uint64_t lock;
};
void pushcli();
void popcli();
void acquire(struct spinlock* s);
void release(struct spinlock* s);