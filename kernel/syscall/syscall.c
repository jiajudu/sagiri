#include<syscall/syscall.h>
#include<lib/x64.h>
#include<lib/stdio.h>
#include<proc/proc.h>
#include<dev/console.h>
#include<debug/debug.h>
#include<proc/cpu.h>
#include<fs/fs.h>
#include<lib/string.h>
void sysenter();
uint64_t sys_fork(uint64_t arg0, uint64_t arg1, uint64_t arg2, uint64_t arg3, uint64_t arg4){
    return fork();
}
uint64_t sys_exit(uint64_t arg0, uint64_t arg1, uint64_t arg2, uint64_t arg3, uint64_t arg4){
    exitproc(arg0);
    return 0;
}
uint64_t sys_thread(uint64_t arg0, uint64_t arg1, uint64_t arg2, uint64_t arg3, uint64_t arg4){
    return userthread(arg0, arg1);
}
uint64_t sys_threadexit(uint64_t arg0, uint64_t arg1, uint64_t arg2, uint64_t arg3, uint64_t arg4){
    exitthread(arg0);
    return 0;
}
uint64_t sys_waitproc(uint64_t arg0, uint64_t arg1, uint64_t arg2, uint64_t arg3, uint64_t arg4){
    if(arg1 > 0x800000000000){
        return -1;
    }
    int64_t ret = 0;
    int64_t retv = 0;
    ret = waitproc(arg0, &retv);
    *((int64_t*)arg1) = retv;
    return ret;
}
uint64_t sys_waitthread(uint64_t arg0, uint64_t arg1, uint64_t arg2, uint64_t arg3, uint64_t arg4){
    if(arg1 > 0x800000000000){
        return -1;
    }
    int64_t ret = 0;
    int64_t retv = 0;
    ret = waitthread(arg0, &retv);
    *((int64_t*)arg1) = retv;
    return ret;
}
uint64_t sys_getpid(uint64_t arg0, uint64_t arg1, uint64_t arg2, uint64_t arg3, uint64_t arg4){
    return getpid();
}
uint64_t sys_gettid(uint64_t arg0, uint64_t arg1, uint64_t arg2, uint64_t arg3, uint64_t arg4){
    return gettid();
}
uint64_t sys_killproc(uint64_t arg0, uint64_t arg1, uint64_t arg2, uint64_t arg3, uint64_t arg4){
    return killproc(arg0);
}
uint64_t sys_killthread(uint64_t arg0, uint64_t arg1, uint64_t arg2, uint64_t arg3, uint64_t arg4){
    return killthread(arg0);
}
uint64_t sys_sleep(uint64_t arg0, uint64_t arg1, uint64_t arg2, uint64_t arg3, uint64_t arg4){
    sleep(arg0 * cpuno);
    return 0;
}
uint64_t sys_open(uint64_t arg0, uint64_t arg1, uint64_t arg2, uint64_t arg3, uint64_t arg4){
    char* name = (char*)arg0;
    if(strncmp(name, ":console", 12) == 0){
        return consoleopen(arg1);
    }else{
        return fileopen((char*)arg0, arg1);
    }
}
uint64_t sys_close(uint64_t arg0, uint64_t arg1, uint64_t arg2, uint64_t arg3, uint64_t arg4){
    if(arg0 >= 16){
        return -1;
    }
    if(cpu->thread->proc->pfdtable[arg0] != 0 && cpu->thread->proc->pfdtable[arg0]->isconsole){
        return consoleclose(arg0);
    }
    return fileclose(arg0);
}
uint64_t sys_read(uint64_t arg0, uint64_t arg1, uint64_t arg2, uint64_t arg3, uint64_t arg4){
    if(arg0 >= 16){
        return -1;
    }
    if(cpu->thread->proc->pfdtable[arg0] != 0 && cpu->thread->proc->pfdtable[arg0]->isconsole){
        return consoleread(arg0, (char*)arg1, arg2);
    }
    return fileread(arg0, (char*)arg1, arg2);
}
uint64_t sys_write(uint64_t arg0, uint64_t arg1, uint64_t arg2, uint64_t arg3, uint64_t arg4){
    if(arg0 >= 16){
        return -1;
    }
    if(cpu->thread->proc->pfdtable[arg0] != 0 && cpu->thread->proc->pfdtable[arg0]->isconsole){
        return consolewrite(arg0, (char*)arg1, arg2);
    }
    return filewrite(arg0, (char*)arg1, arg2);
}
uint64_t sys_unlink(uint64_t arg0, uint64_t arg1, uint64_t arg2, uint64_t arg3, uint64_t arg4){
    return fileunlink((char*)arg0);
}
uint64_t sys_readdir(uint64_t arg0, uint64_t arg1, uint64_t arg2, uint64_t arg3, uint64_t arg4){
    return filereaddir((char*)arg0, (struct dircontent*)arg1);
}
uint64_t sys_stat(uint64_t arg0, uint64_t arg1, uint64_t arg2, uint64_t arg3, uint64_t arg4){
    return filestat((char*)arg0, (struct stat*)arg1);
}
uint64_t sys_mkdir(uint64_t arg0, uint64_t arg1, uint64_t arg2, uint64_t arg3, uint64_t arg4){
    return filemkdir((char*)arg0);
}
uint64_t sys_rmdir(uint64_t arg0, uint64_t arg1, uint64_t arg2, uint64_t arg3, uint64_t arg4){
    return filermdir((char*)arg0);
}
uint64_t sys_lseek(uint64_t arg0, uint64_t arg1, uint64_t arg2, uint64_t arg3, uint64_t arg4){
    if(arg0 >= 16 || arg2 >= 3){
        return -1;
    }
    return fileseek(arg0, arg1, arg2);
}
uint64_t sys_exec(uint64_t arg0, uint64_t arg1, uint64_t arg2, uint64_t arg3, uint64_t arg4){
    return exec((char*)arg0, (uint64_t*)arg1);
}
uint64_t (*systable[32])(uint64_t, uint64_t, uint64_t, uint64_t, uint64_t) = {
    sys_fork, sys_exit, sys_thread, sys_threadexit, sys_waitproc, sys_waitthread, sys_getpid, sys_gettid, sys_killproc, sys_killthread, sys_sleep,
    sys_open, sys_close, sys_read, sys_write, sys_unlink, sys_readdir, sys_stat, sys_mkdir, sys_rmdir, sys_lseek,
    sys_exec
};
void syscall(struct syscallframe* sf){
    cpu->thread->sf = sf;
    sf->rax = systable[sf->rax](sf->rdi, sf->rsi, sf->rdx, sf->r8, sf->r9);
    if(cpu->thread->proc && cpu->thread->proc->killed){
        exitproc(-1);
    }
    if(cpu->thread->killed){
        exitthread(-1);
    }
}
void syscallinit(){
    //设置IA32_FMASK寄存器, 不改变RFLAGS
    wrmsr(0xc0000084, 0, 0);
    //设置IA32_STAR寄存器
    wrmsr(0xc0000081, 0, (24 << 16) | 8);
    //设置IA32_LSTAR寄存器
    uint64_t rip = (uint64_t)sysenter;
    wrmsr(0xc0000082, rip & 0xffffffff, (rip >> 32) & 0xffffffff);
    //设置syscall发生时禁用中断
    wrmsr(0xc0000084, 0x200, 0);
}