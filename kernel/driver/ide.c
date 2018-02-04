#include<driver/ide.h>
#include<sync/spinlock.h>
#include<lib/x64.h>
#include<lib/stdio.h>
#include<dev/console.h>
static struct spinlock idelock;
//读取扇区secno到地址addr处
void readsect(uint64_t addr, uint64_t secno) {
    acquire(&idelock);
    while((inb(0x1f7) & 0xc0) != 0x40);
    outb(0x1f2, 1);
    outb(0x1f3, secno & 0xff);
    outb(0x1f4, (secno >> 8) & 0xff);
    outb(0x1f5, (secno >> 16) & 0xff);
    outb(0x1f6, ((secno >> 24) & 0xf) | 0xf0);
    outb(0x1f7, 0x20);
    while((inb(0x1f7) & 0xc0) != 0x40);
    insl(0x1f0, (void*)addr, 128);
    release(&idelock);
}
//把地址addr处的数据写入扇区secno
void writesect(uint64_t addr, uint64_t secno) {
    acquire(&idelock);
    while((inb(0x1f7) & 0xc0) != 0x40);
    outb(0x1f2, 1);
    outb(0x1f3, secno & 0xff);
    outb(0x1f4, (secno >> 8) & 0xff);
    outb(0x1f5, (secno >> 16) & 0xff);
    outb(0x1f6, ((secno >> 24) & 0xf) | 0xf0);
    outb(0x1f7, 0x30);
    outsl(0x1f0, (void*)addr, 128);
    while((inb(0x1f7) & 0xc0) != 0x40);
    release(&idelock);
}
