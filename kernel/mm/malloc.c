#include<mm/malloc.h>
#include<lib/stdio.h>
#include<debug/debug.h>
uint64_t freeblocks = 0; //空闲内存页数
uint64_t freememory = 0; //空闲页链表头
uint64_t alloc(){
    //申请一页内存
    uint64_t ret = 0;
    if(freememory != 0){
        freeblocks--;
        ret = freememory;
        freememory = *((uint64_t*)freememory);
    }
    return ret;
}
void free(uint64_t p){
    //释放一页内存
    freeblocks++;
    *((uint64_t*)p) = freememory;
    freememory = p;
}
uint64_t mfreememory = 0; //malloc分类器的页链表头
void printpage();
static void minitpage(uint64_t cur) {
    //初始化一页
    //每页的结构为512个uint64_t
    uint64_t* p = (uint64_t*)cur;
    p[0] = 0; // 第0个表示下一页的地址
    //页内使内存块组成一个空闲链表. 每个链表节点由一个8B的节点头和之后的一块内存组成. 节点头的结构为:
    //bit11-0: 后面一块内存的大小, bit23-12: 下一节点的偏移量, bit35-24: 上一节点的偏移量, bit47-36: 下一空闲节点的偏移量, bit59-48: 上一空闲节点的偏移量, bit60: 后面一块内存是否被占用
    p[1] = 0x0l | (0x2l << 12) | (0x0l << 24) | (0x2l << 36) | (0x0l << 48) | (0x0l << 60);
    p[2] = 0xfe0l | (0x1ffl << 12) | (0x1l << 24) | (0x1ffl << 36) | (0x1l << 48) | (0x0l << 60);
    p[0x1ff] = 0x0l | (0x0l << 12) | (0x2l << 24) | (0x0l << 36) | (0x2l << 48) | (0x0l << 60);
}
static uint64_t mgetblock(uint64_t cur, uint64_t size){
    //在cur这一页中尝试分配size大小的空间
    size = (size + 7) / 8 * 8;
    uint64_t* p = (uint64_t*)cur;
    uint64_t curblock = (p[1] >> 36) & 0xfff;
    while(curblock != 0x1ff) {
        uint64_t cursize = p[curblock] & 0xfff;
        if(cursize >= size) {
            break;
        }
        curblock = (p[curblock] >> 36) & 0xfff;
    }
    if(curblock == 0x1ff) {
        return 0;
    }else{
        uint64_t csize = p[curblock] & 0xfff;
        if(csize >= size + 16) {
            //分出一个新块
            uint64_t newindex = curblock + size / 8 + 1;
            uint64_t newsize = csize - 8 - size;
            uint64_t next = (p[curblock] >> 12) & 0xfff;
            uint64_t nextf = (p[curblock] >> 36) & 0xfff;
            p[newindex] = newsize | (next << 12) | (curblock << 24) | (nextf << 36) | (curblock << 48) | (0x0l << 60);
            p[curblock] = (p[curblock] & (~(0xfffl << 0))) | (size << 0);
            p[curblock] = (p[curblock] & (~(0xfffl << 12))) | (newindex << 12);
            p[curblock] = (p[curblock] & (~(0xfffl << 36))) | (newindex << 36);
            p[next] = (p[next] & (~(0xfffl << 24))) | (newindex << 24);
            p[nextf] = (p[nextf] & (~(0xfffl << 48))) | (newindex << 48);
        }
        uint64_t prevf = (p[curblock] >> 48) & 0xfff;
        uint64_t nextf = (p[curblock] >> 36) & 0xfff;
        p[curblock] = p[curblock] | (1l << 60);
        p[prevf] = (p[prevf] & (~(0xfffl << 36))) | (nextf << 36);
        p[nextf] = (p[nextf] & (~(0xfffl << 48))) | (prevf << 48);
        return cur + curblock * 8 + 8;
    }
}
static uint64_t mfreeblock(uint64_t cur, uint64_t addr) {
    //在cur这一页中释放addr对应的块
    uint64_t* p = (uint64_t*)cur;
    uint64_t curblock = (addr & 0xfffl) / 8 - 1;
    uint64_t prevf = (p[curblock] >> 48) & 0xfff;
    while(p[prevf] & (1l << 60)){
        prevf = (p[prevf] >> 48) & 0xfff;
    }
    uint64_t nextf = (p[curblock] >> 36) & 0xfff;
    while(p[nextf] & (1l << 60)) {
        nextf = (p[nextf] >> 36) & 0xfff;
    }
    p[prevf] = (p[prevf] & (~(0xfffl << 36))) | (curblock << 36);
    p[nextf] = (p[nextf] & (~(0xfffl << 48))) | (curblock << 48);
    p[curblock] = (p[curblock] & (~(0xfffl << 36))) | (nextf << 36);
    p[curblock] = (p[curblock] & (~(0xfffl << 48))) | (prevf << 48);
    p[curblock] = (p[curblock] & (~(1l << 60)));
    uint64_t prev = (p[curblock] >> 24) & 0xfff;
    uint64_t next = (p[curblock] >> 12) & 0xfff;
    prevf = (p[curblock] >> 48) & 0xfff;
    nextf = (p[curblock] >> 36) & 0xfff;
    if(!(p[prev] & (1l << 60)) && prev != 1){
        p[prev] = (p[prev] & (~(0xfffl << 12))) | (next << 12);
        p[prev] = (p[prev] & (~(0xfffl << 36))) | (nextf << 36);
        p[next] = (p[next] & (~(0xfffl << 24))) | (prev << 24);
        p[nextf] = (p[nextf] & (~(0xfffl << 48))) | (prevf << 48);
        p[prev] = (p[prev] & (~0xfffl)) | ((p[prev] & 0xfffl) + 8 + (p[curblock] & 0xfffl));
    }
    if(!(p[next] & (1l << 60)) && next != 0x1ff){
        curblock = next;
        prev = (p[curblock] >> 24) & 0xfff;
        next = (p[curblock] >> 12) & 0xfff;
        prevf = (p[curblock] >> 48) & 0xfff;
        nextf = (p[curblock] >> 36) & 0xfff;
        p[prev] = (p[prev] & (~(0xfffl << 12))) | (next << 12);
        p[prev] = (p[prev] & (~(0xfffl << 36))) | (nextf << 36);
        p[next] = (p[next] & (~(0xfffl << 24))) | (prev << 24);
        p[nextf] = (p[nextf] & (~(0xfffl << 48))) | (prevf << 48);
        p[prev] = (p[prev] & (~0xfffl)) | ((p[prev] & 0xfffl) + 8 + (p[curblock] & 0xfffl));
    }
    if((p[2] & 0xfff) == 0xfe0) {
        return 1;
    }else{
        return 0;
    }
}
uint64_t malloc(uint64_t size) {
    //申请一小块内存, 先在malloc分类器的内存链表中查找是否有可用的块, 如果没有, 就申请一页. 每一页的前8B表示下一页的地址
    if(size > 0xfe0) {
        return 0;
    }
    uint64_t prev = 0;
    uint64_t cur = mfreememory;
    while(cur != 0) {
        uint64_t ret = mgetblock(cur, size);
        if(ret != 0) {
            return ret;
        }
        prev = cur;
        cur = *((uint64_t*)cur);
    }
    uint64_t newpage = alloc();
    if(newpage == 0) {
        return 0;
    }
    if(prev != 0){
        *((uint64_t*)prev) = newpage;
    }else{
        mfreememory = newpage;
    }
    cur = newpage;
    minitpage(cur);
    uint64_t ret = mgetblock(cur, size);
    return ret;
}
void mfree(uint64_t p) {
    uint64_t prev = 0;
    uint64_t cur = mfreememory;
    while(cur != 0) {
        if(cur == (p & 0xfffffffffffff000)) {
            uint64_t empty = mfreeblock(cur, p);
            if(empty) {
                if(prev == 0) {
                    uint64_t next = *((int64_t*)mfreememory);
                    free(mfreememory);
                    mfreememory = next;
                }else{
                    *((uint64_t*)prev) = *((uint64_t*)cur);
                    free(cur);
                }
            }
            return;
        }
        prev = cur;
        cur = *((uint64_t*)cur);
    }
}
void printblock(uint64_t cur){
    printf("|-page@%x\n", cur);
    uint64_t* p = (uint64_t*)cur;
    uint64_t c = 2;
    while(c != 0x1ff){
        uint64_t size = p[c] & 0xfffl;
        uint64_t next = (p[c] >> 12) & 0xfffl;
        uint64_t used = (p[c] >> 60) & 0x1l;
        printf("| |-block@%x size %d ", cur + c * 8, size);
        if(used){
            printf("used\n");
        }else{
            printf("free\n");
        }
        c = next;
    } 
}
void printpage() {
    printf("print page:\n");
    uint64_t cur = mfreememory;
    while(cur != 0) {
        printblock(cur);
        cur = *((uint64_t*)cur);
    }
    printf("\n");
}
void malloctest(){
    printpage();
    uint64_t a = malloc(15);
    printpage();
    uint64_t b = malloc(25);
    printpage();
    uint64_t c = malloc(35);
    printpage();
    uint64_t d = malloc(4000);
    printpage();
    mfree(b);
    printpage();
    b = malloc(19);
    printpage();
    mfree(d);
    printpage();
    mfree(a);
    printpage();
    mfree(b);
    printpage();
    a = malloc(60);
    printpage();
    mfree(c);
    printpage();
    while(1){
        ;
    }
}