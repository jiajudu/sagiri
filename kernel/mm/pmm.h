#pragma once
#include "lib/stdint.h"
void pmminit();
//探测到的内存布局
struct e820map {
    int nr_map;
    struct {
        uint64_t addr;//内存块基地址
        uint64_t size;//内存大小, 其中0x1表示可用
        uint32_t type;//内存类型
    } __attribute__((packed)) map[20];
};