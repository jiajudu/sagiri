#pragma once
struct trapframe {
    uint64_t rax;
    uint64_t rbx;
    uint64_t rcx;
    uint64_t rdx;
    uint64_t rbp;
    uint64_t rsi;
    uint64_t rdi;
    uint64_t r8;
    uint64_t r9;
    uint64_t r10;
    uint64_t r11;
    uint64_t r12;
    uint64_t r13;
    uint64_t r14;
    uint64_t r15;
    uint64_t trapno;
    uint64_t err;
    uint64_t rip;
    uint64_t cs;
    uint64_t rflags;
    uint64_t rsp;
    uint64_t ss;
};
void idtinit();
extern struct waiter tick;
struct idtentry{
    uint16_t off15_0;
    uint16_t seg;
    uint8_t ist_resv1;
    uint8_t p_dpl_s_type;
    uint16_t off31_16;
    uint32_t off63_32;
    uint32_t resv2;
};
extern struct idtentry idt[256];