#pragma once
#include<lib/util.h>
struct superblock{
    uint64_t size; // 映像的大小
    uint64_t bitmapblock; // bitmap占的块数
    uint64_t inodeblocks; // inode占的块数
    uint64_t datablocks; // 数据占的块数
    uint64_t rootinode; // 第一个inode
};
struct inode{
    uint32_t type; // 0: 普通文件, 1: 符号链接
    uint32_t size; // 文件大小
    uint32_t addr[14]; // 每一块的偏移量. 
};
struct dirent{
    uint32_t inodenum;
    char name[12];
};
void fsinit();
