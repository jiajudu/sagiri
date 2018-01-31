#pragma once
#include<lib/util.h>
// routines to let c code use special x86 instructions.
static inline uint8_t inb(uint16_t port) {
    uint8_t data;
    asm volatile("in %1,%0" : "=a" (data) : "d" (port));
    return data;
}
static inline void insl(int32_t port, void *addr, int32_t cnt) {
    asm volatile("cld; rep insl" : "=D" (addr), "=c" (cnt) : "d" (port), "0" (addr), "1" (cnt) : "memory", "cc");
}
static inline void outb(uint16_t port, uint8_t data) {
    asm volatile("out %0,%1" : : "a" (data), "d" (port));
}
static inline void outw(uint16_t port, uint16_t data) {
    asm volatile("out %0,%1" : : "a" (data), "d" (port));
}
static inline void outsl(int32_t port, const void *addr, int32_t cnt) {
    asm volatile("cld; rep outsl" : "=S" (addr), "=c" (cnt) : "d" (port), "0" (addr), "1" (cnt) : "cc");
}
static inline void stosb(void *addr, int32_t data, int32_t cnt) {
    asm volatile("cld; rep stosb" : "=D" (addr), "=c" (cnt) : "0" (addr), "1" (cnt), "a" (data) : "memory", "cc");
}
static inline void stosl(void *addr, int32_t data, int32_t cnt) {
    asm volatile("cld; rep stosl" : "=D" (addr), "=c" (cnt) : "0" (addr), "1" (cnt), "a" (data) : "memory", "cc");
}
static inline void lgdt(void* p, int32_t size) {
    volatile uint16_t pd[5];
    pd[0] = size-1;
    pd[1] = (uint64_t)p;
    pd[2] = (uint64_t)p >> 16;
    pd[3] = (uint64_t)p >> 32;
    pd[4] = (uint64_t)p >> 48;
    asm volatile("lgdt (%0)" : : "r" (pd));
}
static inline void lidt(void* p, int32_t size) {
    volatile uint16_t pd[5];
    pd[0] = size-1;
    pd[1] = (uint64_t)p;
    pd[2] = (uint64_t)p >> 16;
    pd[3] = (uint64_t)p >> 32;
    pd[4] = (uint64_t)p >> 48;
    asm volatile("lidt (%0)" : : "r" (pd));
}
static inline void ltr(uint16_t sel) {
    asm volatile("ltr %0" : : "r" (sel));
}
static inline uint64_t readeflags(void) {
    uint64_t eflags;
    asm volatile("pushf; pop %0" : "=r" (eflags));
    return eflags;
}
static inline void loadgs(uint16_t v) {
    asm volatile("movw %0, %%gs" : : "r" (v));
}
static inline void cli(void) {
    asm volatile("cli");
}
static inline void sti(void) {
    asm volatile("sti");
}
static inline void hlt(void) {
    asm volatile("hlt");
}
static inline uint32_t xchg(void* addr, uint64_t newval) {
    uint32_t result;
    uint32_t* address = (uint32_t*)addr;
    // the + in "+m" denotes a read-modify-write operand.
    asm volatile("lock; xchgl %0, %1" : "+m" (*address), "=a" (result) : "1" (newval) : "cc");
    return result;
}
static inline uint64_t rcr2(void) {
    uint64_t val;
    asm volatile("mov %%cr2,%0" : "=r" (val));
    return val;
}
static inline void lcr3(uint64_t val) {
    asm volatile("mov %0,%%cr3" : : "r" (val));
}
static inline void wrmsr(uint32_t msr, uint32_t lo, uint32_t hi){
   asm volatile("wrmsr" : : "a"(lo), "d"(hi), "c"(msr));
}
static inline void invlpg(uint64_t addr) {
    asm volatile ("invlpg (%0)" :: "r" (addr) : "memory");
}