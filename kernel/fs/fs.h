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
    uint32_t type; // 1: 普通文件, 2: 目录, 3: 符号链接
    uint32_t size; // 文件大小
    uint32_t addr[14]; // 每一块的偏移量. 
};
struct dirent{
    uint32_t inodenum;
    char name[12];
};
struct dircontent{
    char c[14 * 32][16];
};
struct stat{
    uint64_t type;
    uint64_t size;
};
enum filetype{
    file_unused = 0, file_file = 1, file_directory = 2, file_symbollink = 3
};
void fsinit();
int64_t fileopen(char* name, uint64_t flags);
int64_t fileclose(uint64_t fdn);
int64_t fileread(uint64_t fdn, char* buf, uint64_t size);
int64_t filewrite(uint64_t fdn, char* buf, uint64_t size);
int64_t fileunlink(char* name);
int64_t filereaddir(char* name, struct dircontent* buf);
int64_t filestat(char* name, struct stat* buf);
int64_t filemkdir(char* name);
int64_t filermdir(char* name);