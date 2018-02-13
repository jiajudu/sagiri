#include<dev/console.h>
#include<driver/uart.h>
#include<sync/spinlock.h>
#include<proc/cpu.h>
#include<fs/fs.h>
struct spinlock consolelock;
void consoleput(int64_t c){
    if(systemstarted){
        acquire(&consolelock);
    }
    uartputc(c);
    if(systemstarted){
        release(&consolelock);
    }   
}
struct spinlock consolebuflock;
char consolebuf[512];
static uint64_t start = 0;
static uint64_t end = 0;
void consolewritec(char c){
    acquire(&consolebuflock);
    consolebuf[start] = c;
    start++;
    if(start == 512){
        start = 0;
    }
    release(&consolebuflock);
}
char consolegetc(){
    char ret = -1;
    acquire(&consolebuflock);
    if(start != end){
        ret = consolebuf[end];
        end++;
        if(end == 512){
            end = 0;
        }
    }
    release(&consolebuflock);
    return ret;
}
int64_t consoleopen(uint64_t flags){
    acquire(&fslock);
    int64_t ret = -1;
    uint64_t flag_read = flags & 1;
    uint64_t flag_write = (flags & 2) >> 1;
    if(flag_read == 0 && flag_write == 0){
        release(&fslock);
        return -1;
    }
    for(uint64_t i = 0; i < 16; i++){
        if(cpu->thread->proc->pfdtable[i] == 0){
            ret = i;
            break;
        }
    }
    if(ret < 0){
        release(&fslock);
        return -1;
    }
    struct filedescriptor* fd = allocfiledescriptor();
    if(fd == 0){
        release(&fslock);
        return -1;
    }
    fd->fnode = 0;
    fd->off = 0;
    fd->readable = (flag_read ? 1: 0);
    fd->ref = 1;
    fd->writable = (flag_write ? 1: 0);
    fd->isconsole = 1;
    cpu->thread->proc->pfdtable[ret] = fd;
    release(&fslock);
    return ret;
}
int64_t consoleclose(uint64_t fdn){
    acquire(&fslock);
    struct filedescriptor* fd = cpu->thread->proc->pfdtable[fdn];
    if(fd == 0){
        release(&fslock);
        return -1;
    }
    if(!(fd->isconsole)){
        release(&fslock);
        return -1;
    }
    fd->ref--;
    if(fd->ref == 0){
        freefiledescriptor(fd);
    }
    cpu->thread->proc->pfdtable[fdn] = 0;
    release(&fslock);
    return 0;
}
int64_t consoleread(uint64_t fdn, char* buf, uint64_t size){
    int64_t ret = 0;
    acquire(&fslock);
    struct filedescriptor* fd = cpu->thread->proc->pfdtable[fdn];
    if(fd == 0){
        release(&fslock);
        return -1;
    }
    if(!(fd->isconsole)){
        release(&fslock);
        return -1;
    }
    if(!(fd->readable)){
        release(&fslock);
        return -1;
    }
    release(&fslock);
    while(size > 0){
        char c = consolegetc();
        if(c >= 0){
            *buf = c;
            buf++;
            size--;
            ret++;
        }else{
            break;
        }
    }
    return ret;
}
int64_t consolewrite(uint64_t fdn, char* buf, uint64_t size){
    int64_t ret = 0;
    acquire(&fslock);
    struct filedescriptor* fd = cpu->thread->proc->pfdtable[fdn];
    if(fd == 0){
        release(&fslock);
        return -1;
    }
    if(!(fd->isconsole)){
        release(&fslock);
        return -1;
    }
    if(!(fd->writable)){
        release(&fslock);
        return -1;
    }
    release(&fslock);
    while(size > 0){
        char c = *buf;
        consoleput(c);
        buf++;
        size--;
        ret++;
    }
    return ret;
}