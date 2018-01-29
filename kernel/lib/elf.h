#pragma once
//elf64-x86-64文件头
struct elfhdr {
    uint32_t magic;
    uint8_t elf[12];
    uint16_t type;
    uint16_t machine;
    uint32_t version;
    uint64_t entry;
    uint64_t phoff;
    uint64_t shoff;
    uint32_t flags;
    uint16_t ehsize;
    uint16_t phentsize;
    uint16_t phnum;
    uint16_t shentsize;
    uint16_t shnum;
    uint16_t shstrndx;
};
//elf64-x86-64段表
struct proghdr {
    uint32_t type;
    uint32_t flags;
    uint64_t offset;
    uint64_t va;
    uint64_t pa;
    uint64_t filesz;
    uint64_t memsz;
    uint64_t align;
};