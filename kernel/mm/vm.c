#include<mm/vm.h>
#include<mm/malloc.h>
#include<lib/string.h>
#include<lib/stdio.h>
#include<proc/cpu.h>
#include<sync/spinlock.h>
#include<debug/debug.h>
#include<lib/x64.h>
uint64_t* kpgdir;
const uint64_t pte_p = 0x001;//是否存在
const uint64_t pte_w = 0x002;//是否可写
const uint64_t pte_u = 0x004;//用户是否可用
const uint64_t pte_pwt = 0x008;//write through
const uint64_t pte_pcd = 0x010;//cache-disable
//获取虚拟地址pgdir::va的页表项地址
uint64_t* getptepointer(uint64_t* pgdir, uint64_t va){
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
    return pgdir4 + pt4;
}
//把虚拟地址pgdir::va映射到pa, 权限为flag
void setmap(uint64_t* pgdir, uint64_t va, uint64_t pa, uint64_t flag){
    uint64_t* pte = getptepointer(pgdir, va);
    *pte = pa | flag;
}
//pageref保存了物理内存中每页被用户进程引用的次数
uint64_t* pagerefdir;
struct spinlock pagereflock;
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
void pagefault(uint64_t addr, uint64_t err, struct trapframe* tf){
    addr = addr / 0x1000 * 0x1000;
    acquire(&(cpu->thread->proc->pgdirlock));
    if(cpu->thread->proc == 0){
        panic("page fault error\n");
    }
    if((addr >= 0x400000 && addr < cpu->thread->proc->heaptop) || (addr >= cpu->thread->proc->stacktop && addr < 0x800000000000)){
        if(err & 1){
            acquire(&pagereflock);
            uint64_t* pte = getptepointer(cpu->thread->proc->pgdir, addr);
            uint64_t oldpa = *pte & 0xfffffffffffff000;
            uint64_t* oldparefp = getpagerefpointer(oldpa);
            if(*oldparefp > 1){
                uint64_t newpa = k2p(alloc());
                if(newpa == 0){
                    panic("mem error");
                }
                memcopy((char*)p2k(newpa), (char*)p2k(oldpa), 4096);
                *pte = newpa | pte_p | pte_u | pte_w;
                *oldparefp -= 1;
                uint64_t* newparefp = getpagerefpointer(newpa);
                assert(*newparefp == 0);
                *newparefp += 1;
            }else{
                *pte = *pte | pte_w;
            }
            release(&pagereflock);
        }else{
            uint64_t p = k2p(alloc());
            setmap(cpu->thread->proc->pgdir, addr, p, pte_p | pte_u | pte_w);
            acquire(&pagereflock);
            uint64_t* paref = getpagerefpointer(p);
            release(&pagereflock);
            *paref = 1;
        }
        invlpg(addr);
    }else{
        release(&(cpu->thread->proc->pgdirlock));
        printf("heaptop = %x\n", cpu->thread->proc->heaptop);
        printf("stacktop = %x\n", cpu->thread->proc->stacktop);
        printf("addr = %x\n", addr);
        printtrapframe(tf);
        printf("killed.\n");
        exitproc(-1);
    }
    release(&(cpu->thread->proc->pgdirlock));
}
void clearusermem(){
    if(cpu->thread->proc == 0){
        panic("clear usermem error\n");
    }
    acquire(&(cpu->thread->proc->pgdirlock));
    acquire(&pagereflock);
    for(uint64_t p = 0x400000; p < cpu->thread->proc->heaptop; p += 0x1000){
        uint64_t* pte = getptepointer(cpu->thread->proc->pgdir, p);
        if(*pte & pte_p){
            uint64_t pa = *pte & 0xfffffffffffff000;
            uint64_t* pagerefp = getpagerefpointer(pa);
            *pagerefp -= 1;
            if(*pagerefp == 0){
                free(p2k(pa));
            }
        }
        *pte = 0;
        invlpg(p);
    }
    for(uint64_t p = cpu->thread->proc->stacktop; p < 0x800000000000; p += 0x1000){
        uint64_t* pte = getptepointer(cpu->thread->proc->pgdir, p);
        if(*pte & pte_p){
            uint64_t pa = *pte & 0xfffffffffffff000;
            uint64_t* pagerefp = getpagerefpointer(pa);
            *pagerefp -= 1;
            if(*pagerefp == 0){
                free(p2k(pa));
            }
        }
        *pte = 0;
        invlpg(p);
    }
    cpu->thread->proc->heaptop = 0x400000;
    cpu->thread->proc->stacktop = 0x800000000000;
    release(&pagereflock);
    release(&(cpu->thread->proc->pgdirlock));
}
void copyusermem(struct proc* from, struct proc* to){
    assert(from != 0);
    assert(to != 0);
    acquire(&(from->pgdirlock));
    acquire(&(to->pgdirlock));
    acquire(&pagereflock);
    to->stacktop = 0x800000000000 - 0x8000;
    to->heaptop = from->heaptop;
    for(uint64_t p = 0x400000; p < to->heaptop; p += 0x1000){
        uint64_t* ptefrom = getptepointer(from->pgdir, p);
        uint64_t* pteto = getptepointer(to->pgdir, p);
        if(*ptefrom & pte_p){
            uint64_t pa = *ptefrom & 0xfffffffffffff000;
            *ptefrom = *ptefrom & (~pte_w);
            *pteto = *ptefrom;
            uint64_t* pref = getpagerefpointer(pa);
            *pref += 1;
            invlpg(p);
        }
    }
    uint64_t currentrsp = cpu->thread->sf->r10;
    currentrsp = currentrsp / 0x8000 * 0x8000;
    for(uint64_t spoff = 0; spoff < 0x8000; spoff += 0x1000){
        uint64_t* ptefrom = getptepointer(from->pgdir, currentrsp + spoff);
        uint64_t* pteto = getptepointer(to->pgdir, to->stacktop + spoff);
        if(*ptefrom & pte_p){
            uint64_t pa = *ptefrom & 0xfffffffffffff000;
            *ptefrom = *ptefrom & (~pte_w);
            *pteto = *ptefrom;
            uint64_t* pref = getpagerefpointer(pa);
            *pref += 1;
            invlpg(currentrsp + spoff);
        }
    }
    release(&pagereflock);
    release(&(to->pgdirlock));
    release(&(from->pgdirlock));
}