typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;
typedef unsigned long long uint64_t;
//从端口port中读取一个字节
static inline uint8_t inb(uint16_t port) {
    uint8_t data;
    asm volatile ("inb %1, %0" : "=a" (data) : "d" (port) : "memory");
    return data;
}
//从端口port中读取cnt*4个字节并写入以addr开始的地址中
static inline void insl(uint16_t port, uint32_t addr, uint32_t cnt) {
    asm volatile (
        "cld;"
        "repne; insl;"
        : "=D" (addr), "=c" (cnt)
        : "d" (port), "0" (addr), "1" (cnt)
        : "memory", "cc");
}
//向端口port中写入一个字节
static inline void outb(uint16_t port, uint8_t data) {
    asm volatile ("outb %0, %1" :: "a" (data), "d" (port) : "memory");
}
//elf64-x86-64文件头
struct elfhdr {
    uint32_t e_magic;
    uint8_t e_elf[12];
    uint16_t e_type;
    uint16_t e_machine;
    uint32_t e_version;
    uint64_t e_entry;
    uint64_t e_phoff;
    uint64_t e_shoff;
    uint32_t e_flags;
    uint16_t e_ehsize;
    uint16_t e_phentsize;
    uint16_t e_phnum;
    uint16_t e_shentsize;
    uint16_t e_shnum;
    uint16_t e_shstrndx;
};
//elf64-x86-64段表
struct proghdr {
    uint32_t p_type;
    uint32_t p_flags;
    uint64_t p_offset;
    uint64_t p_va;
    uint64_t p_pa;
    uint64_t p_filesz;
    uint64_t p_memsz;
    uint64_t p_align;
};
//读取扇区secno到地址dst处
static void readsect(uint32_t dst, uint32_t secno) {
    //等待磁盘就绪
    while((inb(0x1F7) & 0xC0) != 0x40);
    //读取一个扇区
    outb(0x1F2, 1);
    outb(0x1F3, secno & 0xFF);
    outb(0x1F4, (secno >> 8) & 0xFF);
    outb(0x1F5, (secno >> 16) & 0xFF);
    outb(0x1F6, ((secno >> 24) & 0xF) | 0xE0);
    //发出读扇区命令
    outb(0x1F7, 0x20);
    //等待磁盘就绪
    while((inb(0x1F7) & 0xC0) != 0x40);
    //读取扇区
    insl(0x1F0, dst, 128);
}
//从kernel文件偏移offset中读取count个字节到地址pa
static void readseg(uint32_t pa, uint32_t count, uint32_t offset) {
    uint32_t end_pa = pa + count;
    pa -= offset % 512;
    uint32_t secno = (offset / 512) + 2;
    for (; pa < end_pa; pa += 512, secno ++) {
        readsect(pa, secno);
    }
}
void bootmain(void) {
    //读取enable文件
    readsect(0x7e00, 1);
    //读取kernel文件的前4k
    readseg(0x10000, 4096, 0);
    //判断是否为合法的elf文件
    if (((struct elfhdr *)0x10000)->e_magic == 0x464C457FU) {
        struct proghdr *ph, *eph;
        //读取kernel文件的每一段并加载到正确的位置
        ph = (struct proghdr *)(0x10000 + (uint32_t)((struct elfhdr *)0x10000)->e_phoff);
        eph = ph + (uint32_t)((struct elfhdr *)0x10000)->e_phnum;
        for (; ph < eph; ph ++) {
            readseg(ph->p_pa, ph->p_memsz, ph->p_offset);
        }
        //调用enable, 进入长模式
        ((void (*)(void))(0x7e00))();
    }
}
