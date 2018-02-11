#include<fs/fs.h>
#include<lib/stdio.h>
#include<driver/ide.h>
#include<sync/spinlock.h>
#include<lib/string.h>
#include<debug/debug.h>
#include<mm/malloc.h>
#include<proc/cpu.h>
struct superblock sb;
struct file{
    struct inode inode;
    uint64_t inodenum;
    uint64_t ref;
    struct file* parent;
};
struct filedescriptor{
    struct file* fnode;
    uint64_t readable;
    uint64_t writable;
    uint64_t ref;
    uint64_t off;
    uint64_t used;
};
struct file ftable[512];
struct filedescriptor fdtable[512];
struct spinlock fslock;
struct file* rootfile;
uint64_t* bitmappage;
struct superblock readsuperblock(){
    char buf[512];
    readsect((uint64_t)buf, 0);
    struct superblock sb = *((struct superblock *)buf);
    return sb;
}
uint64_t bitmapvarifyused(uint64_t no){
    uint64_t pageno = (no / 8) / 512;
    uint64_t pageoff = (no / 8) % 512;
    uint8_t* c = (uint8_t*)(bitmappage[pageno]);
    if(c[pageoff] & (1u << (no % 8))){
        return 1;
    }else{
        return 0;
    }
}
uint64_t bitmapgetfreeblock(uint64_t from, uint64_t to){
    for(uint64_t i = from; i < to; i++){
        if(!bitmapvarifyused(i)){
            return i;
        }
    }
    return -1;
}
void bitmapsetused(uint64_t no){
    uint64_t pageno = (no / 8) / 512;
    uint64_t pageoff = (no / 8) % 512;
    uint8_t* c = (uint8_t*)(bitmappage[pageno]);
    c[pageoff] |= (1u << (no % 8));
    writesect(bitmappage[pageno], 1 + pageno);
}
void bitmapsetunused(uint64_t no){
    uint64_t pageno = (no / 8) / 512;
    uint64_t pageoff = (no / 8) % 512;
    uint8_t* c = (uint8_t*)(bitmappage[pageno]);
    c[pageoff] &= (~(1u << (no % 8)));
    writesect(bitmappage[pageno], 1 + pageno);
}
struct inode getinode(uint64_t inodenum){
    uint64_t no = inodenum / 8 + 1 + sb.bitmapblock;
    char buf[512];
    readsect((uint64_t)buf, no);
    struct inode ret = *(((struct inode*)buf) + inodenum % 8);
    return ret;
}
void writeinode(uint64_t inodenum, struct inode* node){
    uint64_t no = inodenum / 8 + 1 + sb.bitmapblock;
    char buf[512];
    readsect((uint64_t)buf, no);
    *(((struct inode*)buf) + inodenum % 8) = *node;
    writesect((uint64_t)buf, no);
}
void getblock(uint64_t pagenum, char* buf){
    uint64_t no = pagenum + 1 + sb.bitmapblock + sb.inodeblocks;
    readsect((uint64_t)buf, no);
}
void writeblock(uint64_t pagenum, char* buf){
    uint64_t no = pagenum + 1 + sb.bitmapblock + sb.inodeblocks;
    writesect((uint64_t)buf, no);
}
uint64_t allocblock(){
    uint64_t no = bitmapgetfreeblock(1 + sb.bitmapblock + sb.inodeblocks, 1 + sb.bitmapblock + sb.inodeblocks + sb.datablocks);
    if(no < 0){
        return -1;
    }
    bitmapsetused(no);
    return no - 1 - sb.bitmapblock - sb.inodeblocks;
}
uint64_t allocinode(enum filetype type){
    uint64_t freeblocknum = bitmapgetfreeblock(1 + sb.bitmapblock, 1 + sb.bitmapblock + sb.inodeblocks);
    if(freeblocknum < 0){
        return -1;
    }
    char buf[512];
    readsect((uint64_t)buf, freeblocknum);
    struct inode* node = (struct inode*)buf;
    uint64_t off = -1;
    for(uint64_t i = 0; i < 8; i++){
        if(node[i].type != file_unused){
            off = i;
            break;
        }
    }
    assert(node[off].type == file_unused);
    node[off].type = type;
    node[off].size = 0;
    uint64_t hasfree = 0;
    for(uint64_t i = 0; i < 8; i++){
        if(node[i].type == file_unused){
            hasfree = 1;
            break;
        }
    }
    if(!hasfree){
        bitmapsetused(freeblocknum);
    }
    return (freeblocknum - 1 - sb.bitmapblock) * 8 + off;
}
void freeinode(uint64_t no){
    struct inode node = getinode(no);
    node.type = file_unused;
    writeinode(no, &node);
    bitmapsetunused(1 + sb.bitmapblock + no / 8);
}
struct file* allocfile(){
    for(uint64_t i = 0; i < 512; i++){
        if(ftable[i].inode.type == file_unused){
            ftable[i].inode.type = file_file;
            ftable[i].parent = 0;
            return &(ftable[i]);
        }
    }
    return 0;
}
struct filedescriptor* allocfiledescriptor(){
    for(uint64_t i = 0; i < 512; i++){
        if(fdtable[i].used == 0){
            fdtable[i].used = 1;
            return &(fdtable[i]);
        }
    }
    return 0;
}
void freefile(struct file* f){
    f->inode.type = file_unused;
}
void freefiledescriptor(struct filedescriptor* fd){
    fd->used = 0;
}
struct file* getfileptrfromnum(uint64_t inodenum){
    for(uint64_t i = 0; i < 512; i++){
        if(ftable[i].inode.type != file_unused && ftable[i].inodenum == inodenum){
            return &(ftable[i]);
        }
    }
    return 0;
}
void printfile(uint64_t nodenum){
    struct inode n = getinode(nodenum);
    if(n.type == file_directory){
        uint64_t childnum = n.size / 16;
        printf("directory, inode = %d, childnum = %d\n", nodenum, childnum);
        uint64_t blockno = (childnum + 31) / 32;
        uint64_t currentchild = 0;
        for(uint64_t i = 0; i < blockno; i++){
            struct dirent ds[32];
            getblock(n.addr[i], (char*)ds);
            for(uint64_t c = 0; c < 32; c++){
                printf("    child, inode = %d, name = %s\n", ds[c].inodenum, ds[c].name);
                printfile(ds[c].inodenum);
                currentchild++;
                if(currentchild >= childnum){
                    break;
                }
            }
        }
    }else{
        printf("file, inode = %d, size = %d\n", nodenum, n.size);
    }
}
uint64_t verifyfilename(char* name){
    uint64_t length = 0;
    for(uint64_t i = 0; i < 100; i++){
        if(name[i] == 0){
            length = i;
            break;
        }
    }
    if(length == 100 || length < 2){
        return 0;
    }
    if(name[0] == '/' && name[length - 1] != '/'){
        return 1;
    }else{
        return 0;
    }
}
uint64_t finditemindirectory(struct file* parent, char* name){
    assert(strlen(name) <= 12);
    uint64_t childnum = parent->inode.size / 16;
    uint64_t childptr = 0;
    for(uint64_t i = 0; i < 14; i++){
        uint32_t blockaddr = parent->inode.addr[i];
        char block[512];
        getblock(blockaddr, block);
        for(uint64_t j = 0; j < 32; j++){
            struct dirent* d = (struct dirent*)block + j;
            if(strncmp(d->name, name, 12) == 0){
                return d->inodenum;
            }
            childptr++;
            if(childptr >= childnum){
                return -1;
            }
        }
    }
    return -1;
}
struct file* getparentinode(char* name){
    uint64_t start = 1;
    uint64_t end = 1;
    uint64_t length = strlen(name);
    struct file* cur = rootfile;
    cur->ref += 1;
    assert(cur->inode.type == file_directory);
    while(end < length){
        if(name[end] == '/'){
            char namebuf[20];
            for(uint64_t i = start; i < end; i++){
                namebuf[i - start] = name[i];
            }
            namebuf[end] = 0;
            uint64_t inodenum = finditemindirectory(cur, namebuf);
            if(inodenum == -1){
                return 0;
            }
            struct file* fptr = getfileptrfromnum(inodenum);
            if(fptr == 0){
                fptr = allocfile();
                if(fptr == 0){
                    while(cur){
                        cur->ref--;
                        struct file* parent = cur->parent;
                        if(cur->ref == 0){
                            freefile(cur);
                        }
                        cur = parent;
                    }
                }
                fptr->inode = getinode(inodenum);
                assert(fptr->inode.type == file_directory);
                fptr->inodenum = inodenum;
                fptr->ref = 0;
                fptr->parent = cur;
            }
            fptr->ref++;
            assert(fptr->parent == cur);
            cur = fptr;
            start = end + 1;
        }
        end++;
    }
    return cur;
}
uint64_t createfileindir(struct file* n, char* name){
    assert(strlen(name) <= 12);
    if(n->inode.type != file_directory){
        return -1;
    }
    uint64_t inodenum = allocinode(file_file);
    if(n->inode.size == 14 * 512){
        freeinode(inodenum);
        return -1;
    }
    if(n->inode.size % 512 == 0){
        uint64_t pageno = allocblock();
        if(pageno < 0){
            freeinode(inodenum);
            return -1;
        }
        uint64_t newindex = n->inode.size / 512;
        n->inode.addr[newindex] = pageno;
        writeinode(n->inodenum, &(n->inode));
    }
    uint64_t childnum = n->inode.size / 64;
    uint64_t blockno = childnum / 32;
    uint64_t blockoff = childnum % 32;
    n->inode.size += 16;
    char buf[512];
    getblock(n->inode.addr[blockno], buf);
    struct dirent* d = ((struct dirent*)buf) + blockoff;
    d->inodenum = inodenum;
    strncopy(d->name, name, 12);
    writeblock(n->inode.addr[blockno], buf);
    return inodenum;
}
void truncfile(uint64_t inodenum){
    struct inode node = getinode(inodenum);
    uint64_t blocknum = (node.size + 511) / 512;
    assert(blocknum <= 14);
    for(uint64_t i = 0; i < blocknum; i++){
        bitmapsetunused(1 + sb.bitmapblock + sb.inodeblocks + node.addr[i]);
    }
    node.size = 0;
    writeinode(inodenum, &node);
}
int64_t fileopen(char* name, uint64_t flags){
    int64_t ret = -1;
    acquire(&fslock);
    printf("name: %s, flag: %d\n", name, flags);
    uint64_t flag_read = flags & 1;
    uint64_t flag_write = (flags & 2) >> 1;
    uint64_t flag_trunc = (flags & 4) >> 2;
    if(flag_write == 0){
        flag_trunc = 0;
    }
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
    struct file* fn = getparentinode(name);
    if(fn == 0){
        freefiledescriptor(fd);
        release(&fslock);
        return -1;
    }
    char namebuf[20];
    uint64_t namestart = 0;
    uint64_t ptr = 0;
    while(name[ptr] != 0){
        if(name[ptr] == '/'){
            namestart = ptr + 1;
        }
        ptr++;
    }
    for(ptr = namestart; name[ptr] != 0; ptr++){
        namebuf[ptr - namestart] = name[ptr];
    }
    namebuf[ptr] = 0;
    uint64_t inodenum = finditemindirectory(fn, namebuf);
    if(inodenum < 0 && flag_write){
        inodenum = createfileindir(fn, namebuf);
    }
    if(inodenum < 0){
        while(fn != 0){
            fn->ref--;
            struct file* pa = fn->parent;
            if(fn->ref == 0){
                freefile(fn);
            }
            fn = pa;
        }
        freefiledescriptor(fd);
        release(&fslock);
        return -1;
    }
    struct file* f = getfileptrfromnum(inodenum);
    if(f == 0){
        f = allocfile();
        if(f == 0){
            while(fn != 0){
                fn->ref--;
                struct file* pa = f->parent;
                if(fn->ref == 0){
                    freefile(fn);
                }
                fn = pa;
            }
            freefiledescriptor(fd);
            release(&fslock);
            return -1;
        }
        f->inode = getinode(inodenum);
        f->ref = 0;
        f->parent = fn;
    }
    f->ref++;
    assert(f->parent == fn);
    fd->fnode = f;
    fd->off = 0;
    fd->readable = (flag_read ? 1: 0);
    fd->ref = 1;
    fd->writable = (flag_write ? 1: 0);
    cpu->thread->proc->pfdtable[ret] = fd;
    if(flag_trunc){
        truncfile(inodenum);
    }
    release(&fslock);
    return ret;
}
void fsinit(){
    sb = readsuperblock();
    for(uint64_t i = 0; i < 512; i++){
        ftable[i].inode.type = file_unused;
    }
    for(uint64_t i = 0; i < 512; i++){
        fdtable[i].used = 0;
    }
    rootfile = allocfile();
    rootfile->ref++;
    rootfile->inode = getinode(sb.rootinode);
    assert(rootfile->inode.type == file_directory);
    rootfile->inodenum = sb.rootinode;
    rootfile->parent = 0;
    bitmappage = (uint64_t*)alloc();
    for(uint64_t i = 0; i < sb.bitmapblock; i++){
        bitmappage[i] = alloc();
        readsect(bitmappage[i], 1 + i);
    }
}