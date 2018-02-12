#include<proc/proc.h>
#include<proc/cpu.h>
#include<mm/vm.h>
#include<mm/malloc.h>
#include<sync/spinlock.h>
#include<lib/string.h>
#include<lib/stdio.h>
#include<debug/debug.h>
#include<lib/x64.h>
#include<proc/schedule.h>
#include<trap/trap.h>
#include<syscall/syscall.h>
#include<lib/elf.h>
#include<fs/fs.h>
struct context{
    uint64_t r15;
    uint64_t r14;
    uint64_t r13;
    uint64_t r12;
    uint64_t rbx;
    uint64_t rbp;
    uint64_t rip;
};
struct proc procs[128];
struct thread threads[256];
struct spinlock ptablelock;
void kthread();
void forkret();
extern char bspstack[4096];
static void setidleprocess(){
    //把0号进程和0-(cpuno-1)号线程设置为空闲进程/线程
    procs[0].pgdir = kpgdir;
    procs[0].state = proc_running;
    for(uint64_t i = 0; i < cpuno; i++){
        if(cpu == &(cpus[i])){
            threads[i].kstack = (uint64_t*)bspstack;
        }
        threads[i].proc = &(procs[0]);
        threads[i].state = thread_running;
        cpus[i].thread = &(threads[i]);
    }
}
int64_t allocthread(uint64_t newproc){
    acquire(&ptablelock);
    uint64_t* pgdir = 0;
    if(newproc){
        pgdir = (uint64_t*)alloc();
        if(pgdir == 0){
            return -1;
        }
    }
    struct proc* p = 0;
    if(newproc){
        uint64_t curpid = cpu->thread->proc->pid;
        for(uint64_t i = curpid; i < 128; i++){
            if(procs[i].state == proc_unused){
                procs[i].state = proc_running;
                p = &(procs[i]);
                break;
            }
        }
        if(!p){
            for(uint64_t i = 0; i < curpid; i++){
                if(procs[i].state == proc_unused){
                    procs[i].state = proc_running;
                    p = &(procs[i]);
                    break;
                }
            }
        }
        if(p == 0){
            free((uint64_t)pgdir);
            return -1;
        }
        for(uint64_t i = 0; i < 16; i++){
            p->pfdtable[i] = 0;
        }
    }else{
        p = cpu->thread->proc;
    }
    struct thread* t = 0;
    uint64_t curtid = cpu->thread->tid;
    for(uint64_t i = curtid; i < 256; i++){
        if(!(threads[i].state)){
            threads[i].state = thread_sleeping;
            t = &(threads[i]);
            break;
        }
    }
    if(!t){
        for(uint64_t i = 0; i < curtid; i++){
            if(!(threads[i].state)){
                threads[i].state = thread_sleeping;
                t = &(threads[i]);
                break;
            }
        }
    }
    if(t == 0){
        if(newproc){
            p->state = proc_unused;
            free((uint64_t)pgdir);
        }
        return -1;
    }
    if(newproc){
        p->heaptop = 0;
        p->pgdir = pgdir;
        memset((char*)pgdir, 0, 4096);
        memcopy((char*)pgdir + 2048, (char*)kpgdir + 2048, 2048);
        p->stacktop = 0x0000800000000000;
        p->killed = 0;
    }
    t->proc = p;
    t->killed = 0;
    t->needschedule = 0;
    memset((char*)t->kstack + 0x1000, 0, 0x7000);
    release(&ptablelock);
    return t->tid;
}
int64_t createthread(uint64_t (*fn)(void *), void *args, uint64_t newproc){
    int64_t newtid = allocthread(newproc);
    if(newtid < 0){
        return newtid;
    }
    struct thread* t = &(threads[newtid]);
    struct context c;
    c.r15 = (uint64_t)fn;
    c.rbx = (uint64_t)args;
    c.rip = (uint64_t)kthread;
    t->rsp = (uint64_t)t->kstack + 0x8000 - sizeof(struct context);
    memcopy((char*)(t->rsp), (char*)(&c), sizeof(struct context));
    if(!newproc){
        t->proc = cpu->thread->proc;
    }else{
        t->proc->parent = cpu->thread->proc;
    }
    t->state = thread_runnable;
    return 0;
}
int64_t fork(){
    int64_t newtid = allocthread(1);
    if(newtid < 0){
        return -1;
    }
    acquire(&ptablelock);
    struct thread* t = &(threads[newtid]);
    struct proc* p = threads[newtid].proc;
    copyusermem(cpu->thread->proc, p);
    struct context c;
    c.rip = (uint64_t)forkret;
    t->rsp = (uint64_t)t->kstack + 0x8000 - sizeof(struct context) - sizeof(struct syscallframe);
    t->ustack = 0;
    memcopy((char*)(t->rsp), (char*)(&c), sizeof(struct context));
    struct syscallframe* sf = (struct syscallframe*)(t->rsp + sizeof(struct context));
    memcopy((char*)sf, (char*)cpu->thread->sf, sizeof(struct syscallframe));
    sf->rax = 0;
    p->parent = cpu->thread->proc;
    t->state = thread_runnable;
    int64_t ret = p->pid;
    for(uint64_t i = 0; i < 16; i++){
        p->pfdtable[i] = cpu->thread->proc->pfdtable[i];
    }
    release(&ptablelock);
    return ret;
}
int64_t userthread(uint64_t fn, uint64_t args){
    int64_t newtid = allocthread(0);
    if(newtid < 0){
        return -1;
    }
    acquire(&ptablelock);
    struct thread* t = &(threads[newtid]);
    struct proc* p = cpu->thread->proc;
    struct context c;
    c.rip = (uint64_t)forkret;
    t->rsp = (uint64_t)t->kstack + 0x8000 - sizeof(struct context) - sizeof(struct syscallframe);
    t->ustack = 0;
    uint64_t dup = 1;
    while(dup){
        dup = 0;
        for(uint64_t i = 0; i < 256; i++){
            if(threads[i].proc == p && threads[i].ustack == t->ustack && i != t->tid){
                dup = 1;
                t->ustack++;
                break;
            }
        }
    }
    memcopy((char*)(t->rsp), (char*)(&c), sizeof(struct context));
    struct syscallframe* sf = (struct syscallframe*)(t->rsp + sizeof(struct context));
    sf->r11 = readeflags();
    sf->r10 = 0x800000000000 - t->ustack * 0x8000 - 0x20;
    if(sf->r10 < p->stacktop){
        p->stacktop -= 0x8000;
    }
    sf->rcx = fn;
    sf->rdi = args;
    t->state = thread_runnable;
    release(&ptablelock);
    return newtid;
}
void exitthread(int64_t retvalue){
    acquire(&ptablelock);
    cpu->thread->retvalue = retvalue;
    cpu->thread->state = thread_zombie;
    cpu->thread->killed = 0;
    for(uint64_t i = 0; i < 256; i++){
        if(threads[i].waiter == &(cpu->thread->exitwaiter)){
            threads[i].state = thread_runnable;
            threads[i].waiter = 0;
        }
    }
    uint64_t exitproc = 1;
    for(uint64_t i = 0; i < 256; i++){
        if(threads[i].proc == cpu->thread->proc && threads[i].state != thread_zombie){
            exitproc = 0;
        }
    }
    if(exitproc){
        release(&ptablelock);
        for(uint64_t i = 0; i < 16; i++){
            if(cpu->thread->proc->pfdtable[i] != 0){
                fileclose(i);
            }
        }
        acquire(&ptablelock);
        cpu->thread->proc->state = proc_zombie;
        for(uint64_t i = 0; i < 256; i++){
            if(procs[i].parent == cpu->thread->proc){
                procs[i].parent = &(procs[0]);
            }
        }
        clearusermem();
        for(uint64_t i = 0; i < 256; i++){
            if(threads[i].waiter == &(cpu->thread->proc->exitwaiter)){
                threads[i].state = thread_runnable;
                threads[i].waiter = 0;
            }
        }
    }
    release(&ptablelock);
    schedule();
}
void exitproc(int64_t retvalue){
    acquire(&ptablelock);
    cpu->thread->proc->killed = 0;
    for(uint64_t i = 0; i < 256; i++){
        if(threads[i].proc == cpu->thread->proc){
            threads[i].killed = 1;
        }
    }
    cpu->thread->proc->state = proc_exiting;
    cpu->thread->proc->retvalue = retvalue;
    release(&ptablelock);
    schedule();
}
void cleanproc(struct proc* proc){
    for(uint64_t i = 0; i < 256; i++){
        if(threads[i].proc == proc){
            threads[i].proc = 0;
            threads[i].state = thread_unused;
            threads[i].rsp = 0;
            threads[i].killed = 0;
            threads[i].exitwaiter.space = 0;
            threads[i].waiter = 0;
        }
    }
    proc->heaptop = 0;
    proc->parent = 0;
    free((uint64_t)(proc->pgdir));
    proc->pgdir = 0;
    proc->state = proc_unused;
    proc->stacktop = 0x0000800000000000;
    proc->killed = 0;
    proc->exitwaiter.space = 0;
    proc->pgdirlock.lock = 0;
    for(uint64_t i = 0; i < 16; i++){
        proc->pfdtable[i] = 0;
    }
}
void cleanthread(struct thread* t){
    t->proc = 0;
    t->state = thread_unused;
    t->rsp = 0;
    t->killed = 0;
    t->exitwaiter.space = 0;
    t->waiter = 0;
    t->ustack = 0;
}
int64_t waitproc(int64_t pid, int64_t* retp){
    acquire(&ptablelock);
    if(pid == 0 || procs[pid].parent != cpu->thread->proc){
        release(&ptablelock);
        return -1;
    }
    if(procs[pid].state != proc_zombie){
        cpu->thread->waiter = &(procs[pid].exitwaiter);
        cpu->thread->state = thread_sleeping;
        release(&ptablelock);
        schedule();
        acquire(&ptablelock);
        assert(procs[pid].state == proc_zombie);
    }
    int64_t ret = procs[pid].retvalue;
    cleanproc(&(procs[pid]));
    *retp = ret;
    release(&ptablelock);
    return pid;
}
int64_t waitthread(int64_t tid, int64_t* retp){
    acquire(&ptablelock);
    if(threads[tid].proc != cpu->thread->proc || tid < cpuno){
        release(&ptablelock);
        return -1;
    }
    if(threads[tid].state != thread_zombie){
        cpu->thread->waiter = &(threads[tid].exitwaiter);
        cpu->thread->state = thread_sleeping;
        release(&ptablelock);
        schedule();
        acquire(&ptablelock);
        assert(threads[tid].state == thread_zombie);
    }
    int64_t ret = threads[tid].retvalue;
    cleanthread(&(threads[tid]));
    *retp = ret;
    release(&ptablelock);
    return tid;
}
void setkill(uint64_t pid){
    procs[pid].killed = 1;
    for(uint64_t i = 0; i < 256; i++){
        if(procs[i].parent == &(procs[pid])){
            setkill(i);
        }
    }
}
int64_t killproc(uint64_t pid){
    int64_t ret = 0;
    acquire(&ptablelock);
    if(pid != 0){
        setkill(pid);
    }else{
        ret = -1;
    }
    release(&ptablelock);
    return ret;
}
int64_t killthread(uint64_t tid){
    int64_t ret = 0;
    acquire(&ptablelock);
    if(tid >= cpuno && threads[tid].proc == cpu->thread->proc){
        threads[tid].killed = 1;
    }else{
        ret = -1;
    }
    release(&ptablelock);
    return ret;
}
uint64_t getpid(){
    uint64_t ret = 0;
    acquire(&ptablelock);
    ret = cpu->thread->proc->pid;
    release(&ptablelock);
    return ret;
}
uint64_t gettid(){
    uint64_t ret = 0;
    acquire(&ptablelock);
    ret = cpu->thread->tid;
    release(&ptablelock);
    return ret;
}
void sleep(uint64_t ticks){
    acquire(&ptablelock);
    cpu->thread->state = thread_sleeping;
    cpu->thread->waiter = &tick;
    cpu->thread->tick = ticks;
    release(&ptablelock);
    schedule();
}
void proctick(){
    acquire(&ptablelock);
    for(uint64_t i = 0; i < 256; i++){
        if(threads[i].state == thread_sleeping && threads[i].waiter == &(tick)){
            threads[i].tick--;
            if(threads[i].tick == 0){
                threads[i].state = thread_runnable;
                threads[i].waiter = 0;
            }
        }
        if(threads[i].state == thread_running){
            if(threads[i].tick > 0){
                threads[i].tick--;
            }
            if(threads[i].tick == 0){
                threads[i].needschedule = 1;
            }
        }
    }
    release(&ptablelock);
}
int64_t exec(char* name, uint64_t* args){
    printf("exec: %s\n", name);
    int64_t fd = fileopen(name, 1);
    if(fd < 0){
        return -1;
    }
    uint64_t tmp = alloc();
    if(tmp == 0){
        fileclose(fd);
        return -1;
    }
    uint64_t totalspace = 0;
    uint64_t argnum = 0;
    for(uint64_t i = 0; args[i] != 0; i++){
        argnum++;
        totalspace += (strlen((char*)args[i]) + 1);
    }
    char* argstart = (char*)tmp + 0xfe0 - totalspace;
    uint64_t* argpstart = (uint64_t*)(((uint64_t)argstart & 0xfffffffffffffff0) - 8 * argnum);
    for(uint64_t i = 0; args[i] != 0; i++){
        argpstart[i] = (uint64_t)argstart - tmp + 0x7ffffffff000;
        memcopy(argstart, (char*)(args[i]), strlen((char*)(args[i])) + 1);
        argstart += (strlen((char*)(args[i])) + 1);
    }
    for(uint64_t i = 0; i < 256; i++){
        if(threads[i].proc == cpu->thread->proc && cpu->thread != &(threads[i])){
            threads[i].killed = 1;
            int64_t ret = 0;
            waitthread(i, &ret);
        }
    }
    clearusermem();
    struct elfhdr header;
    fileread(fd, (char*)&header, sizeof(struct elfhdr));
    if(header.magic != 0x464c457f){
        fileclose(fd);
        return -1;
    }
    uint64_t maxaddr = 0x400000;
    for(uint64_t i = 0; i < header.phnum; i++){
        struct proghdr ph;
        fileseek(fd, header.phoff + i * sizeof(struct proghdr), 0);
        fileread(fd, (char*)&ph, sizeof(ph));
        if(ph.memsz > 0){
            uint64_t phmaxaddr = (ph.va + ph.memsz + 4095) / 4096 * 4096;
            if(phmaxaddr > maxaddr){
                maxaddr = phmaxaddr;
            }
        }
    }
    cpu->thread->proc->heaptop = maxaddr;
    cpu->thread->proc->stacktop = 0x800000000000 - 0x8000;
    cpu->thread->ustack = 0;
    for(uint64_t i = 0; i < header.phnum; i++){
        struct proghdr ph;
        fileseek(fd, header.phoff + i * sizeof(struct proghdr), 0);
        fileread(fd, (char*)&ph, sizeof(ph));
        if(ph.memsz > 0){
            fileseek(fd, ph.offset, 0);
            fileread(fd, (char*)ph.va, ph.memsz);
        }
    }
    fileclose(fd);
    memcopy((char*)0x7ffffffff000, (char*)tmp, 4096);
    free(tmp);
    struct syscallframe sf;
    sf.rcx = header.entry;
    sf.r10 = (uint64_t)argpstart - tmp + 0x7ffffffff000;
    sf.r11 = readeflags();
    sf.rdi = argnum;
    sf.rsi = (uint64_t)argpstart - tmp + 0x7ffffffff000;
    switchtouser(&sf);
    return 0;
}
uint64_t firstthread(){
    char* arg0 = "/echo";
    char* arg1 = "ksp";
    char* arg2 = "bsesw";
    char* arg3 = "/license";
    uint64_t args[5];
    args[0] = (uint64_t)arg0;
    args[1] = (uint64_t)arg1;
    args[2] = (uint64_t)arg2;
    args[3] = (uint64_t)arg3;
    args[4] = 0;
    exec("/echo", args);
    return 0;
}
void procinit(){
    for(uint64_t i = 0; i < 256; i++){
        threads[i].tid = i;
        threads[i].proc = 0;
        threads[i].state = thread_unused;
        threads[i].rsp = 0;
        threads[i].killed = 0;
        threads[i].exitwaiter.space = 0;
        threads[i].waiter = 0;
        threads[i].needschedule = 0;
        threads[i].kstack = (uint64_t*)(0xffffa00000000000 + 0x10000 * i);
        for(uint64_t j = 1; j < 8; j++){
            uint64_t pa = k2p(alloc());
            setmap(kpgdir, 0xffffa00000000000 + 0x10000 * i + 0x1000 * j, pa, pte_p | pte_w);
        }
    }
    for(uint64_t i = 0; i < 128; i++){
        procs[i].heaptop = 0;
        procs[i].parent = 0;
        procs[i].pgdir = 0;
        procs[i].pid = i;
        procs[i].state = proc_unused;
        procs[i].stacktop = 0x0000800000000000;
        procs[i].killed = 0;
        procs[i].exitwaiter.space = 0;
        procs[i].pgdirlock.lock = 0;
        for(uint64_t j = 0; j < 16; j++){
            procs[i].pfdtable[j] = 0;
        }
    }
    setidleprocess();
    createthread(firstthread, 0, 1);
}
