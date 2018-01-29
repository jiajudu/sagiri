#include<mm/vm.h>
#include<mm/malloc.h>
#include<lib/string.h>
#include<lib/stdio.h>
#include<proc/cpu.h>
#include<sync/spinlock.h>
uint64_t* kpgdir;
const uint64_t pte_p = 0x001;//是否存在
const uint64_t pte_w = 0x002;//是否可写
const uint64_t pte_u = 0x004;//用户是否可用
const uint64_t pte_pwt = 0x008;//write through
const uint64_t pte_pcd = 0x010;//cache-disable
//把虚拟地址pgdir::va映射到pa, 权限为flag
void setmap(uint64_t* pgdir, uint64_t va, uint64_t pa, uint64_t flag){
    uint64_t pt1 = (va >> 39) & 0x1ff;
    uint64_t pt2 = (va >> 30) & 0x1ff;
    uint64_t pt3 = (va >> 21) & 0x1ff;
    uint64_t pt4 = (va >> 12) & 0x1ff;
    if(!(pgdir[pt1] & pte_p)){
        uint64_t dir2 = alloc();
        memset((char*)dir2, 0, 0x1000);
        pgdir[pt1] = k2p(dir2) | pte_p | pte_u | pte_w;
    }
    uint64_t* pgdir2 = (uint64_t*)(p2k(pgdir[pt1] & 0xfffffffffffff000));
    if(!(pgdir2[pt2] & pte_p)){
        uint64_t dir3 = alloc();
        memset((char*)dir3, 0, 0x1000);
        pgdir2[pt2] = k2p(dir3) | pte_p | pte_u | pte_w;
    }
    uint64_t* pgdir3 = (uint64_t*)(p2k(pgdir2[pt2] & 0xfffffffffffff000));
    if(!(pgdir3[pt3] & pte_p)){
        uint64_t dir4 = alloc();
        memset((char*)dir4, 0, 0x1000);
        pgdir3[pt3] = k2p(dir4) | pte_p | pte_u | pte_w;
    }
    uint64_t* pgdir4 = (uint64_t*)(p2k(pgdir3[pt3] & 0xfffffffffffff000));
    pgdir4[pt4] = pa | flag;
}
//pageref保存了物理内存中每页被用户进程引用的次数
uint64_t* pagerefdir;
//获取物理页p被引用次数的位置
uint64_t* getpagerefpointer(uint64_t p){
    uint64_t pt1 = (p >> 39) & 0x1ff;
    uint64_t pt2 = (p >> 30) & 0x1ff;
    uint64_t pt3 = (p >> 21) & 0x1ff;
    uint64_t pt4 = (p >> 12) & 0x1ff;
    if(!(pagerefdir[pt1] & 1)){
        uint64_t dir2 = alloc();
        memset((char*)dir2, 0, 0x1000);
        pagerefdir[pt1] = dir2 | 1;
    }
    uint64_t* pagerefdir2 = (uint64_t*)(pagerefdir[pt1] & 0xfffffffffffff000);
    if(!(pagerefdir2[pt2] & 1)){
        uint64_t dir3 = alloc();
        memset((char*)dir3, 0, 0x1000);
        pagerefdir2[pt2] = dir3 | 1;
    }
    uint64_t* pagerefdir3 = (uint64_t*)(pagerefdir2[pt2] & 0xfffffffffffff000);
    if(!(pagerefdir3[pt3] & 1)){
        uint64_t dir4 = alloc();
        memset((char*)dir4, 0, 0x1000);
        pagerefdir3[pt3] = dir4 | 1;
    }
    uint64_t* pagerefdir4 = (uint64_t*)(pagerefdir3[pt3] & 0xfffffffffffff000);
    return pagerefdir4 + pt4;
}
void pagerefinit(uint64_t maxmem){
    pagerefdir = (uint64_t*)alloc();
    memset((char*)pagerefdir, 0, 0x1000);
    for(uint64_t p = 0; p < maxmem; p += 0x1000) {
        uint64_t* pagerefaddr = getpagerefpointer(p);
        *pagerefaddr = 0;
    }
}
void pagefault(uint64_t addr){
    //printf("pagefault: %x\n", addr);
    addr = addr / 0x1000 * 0x1000;
    acquire(&(cpu->thread->proc->pgdirlock));
    if((addr >= 0x4000 && addr < cpu->thread->proc->heaptop) || (addr >= cpu->thread->proc->stacktop && addr < 0x800000000000)){
        uint64_t p = k2p(alloc());
        setmap(cpu->thread->proc->pgdir, addr, p, pte_p | pte_u | pte_w);
        uint64_t* paref = getpagerefpointer(p);
        *paref = 1;
    }else{
        release(&(cpu->thread->proc->pgdirlock));
        printf("killed.\n");
        exitproc(-1);
    }
    release(&(cpu->thread->proc->pgdirlock));
}