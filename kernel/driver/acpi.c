#include<lib/util.h>
#include<driver/acpi.h>
#include<lib/stdio.h>
#include<mm/mm.h>
#include<lib/string.h>
#include<driver/lapic.h>
#include<driver/ioapic.h>
struct rsdp{
    char signature[8];//必须为"RSD PTR "
    uint8_t checksum;//前20byte的校验和
    char oemid[6];//OEM厂商
    uint8_t revision;//版本信息
    uint32_t rsdtaddress;//rsdt表的物理地址
    uint32_t length;//rsdp的长度
    uint64_t xsdtaddress;//xsdt表的物理地址
    uint8_t extendedchecksum;//全部的检验和
    uint8_t reserved[3];//未使用
} __attribute__((__packed__));
struct sdtheader{
    char signature[4];//签名
    uint32_t length;//结构体的总长度, 包含非头部. 
    uint8_t revision;
    uint8_t checksum;//校验和
    char oemid[6];
    char oemtableid[8];
    uint32_t oemrevision;
    uint32_t creatorid;
    uint32_t creatorrevision;
} __attribute__((__packed__));
//rsdt表存储了其他系统描述符表的信息, 签名为RSDT
struct rsdt{
    struct sdtheader h;
    uint32_t entry[0];
} __attribute__((__packed__));
//madt表描述了所有的中断控制器, 可以用于枚举当前可用的所有处理器. 签名为APIC
struct madt{
    struct sdtheader h;
    uint32_t lapicaddr;
    uint32_t flags;
    uint8_t entry[0];//从这里开始, 存放两种中断控制器的记录, 分别为lapic和ioapic
} __attribute__((__packed__));
struct madtlapic{
    uint8_t type;//为0
    uint8_t length;
    uint8_t acpiid;
    uint8_t apicid;
    uint8_t flags;//bit 1表示是否有效
} __attribute__((__packed__));
struct madtioapic{
    uint8_t type;//为1
    uint8_t length;
    uint8_t ioapicid;
    uint8_t reserved;
    uint32_t ioapicaddr;
    uint32_t globalsysteminterruptbase;
} __attribute__((__packed__));
uint64_t cpuno = 0;
struct cpu cpus[8];
bool issmp = false;
static struct rsdp* searchrsdp(char* begin, char* end){
    for(char* p = begin; p + sizeof(struct rsdp) < end; p += 4){
        if(memcmp(p, "RSD PTR ", 8) == 0){
            uint8_t sum = 0;
            for(uint64_t i = 0; i < sizeof(struct rsdp); i++){
                sum += p[i];
            }
            if(sum == 0){
                return (struct rsdp*)p;
            }
        }
    }
    return nullptr;
} 
//rsdp或者位于EBDA(Extended BIOS Data Area)的前1kB中(EBDA位置为(*(uint16_t*)(0x40e)), 或者在内存0xe0000-0xfffff中. 
static struct rsdp* findrsdp(){
    char* ebda = (char*)p2k((*(uint16_t*)(p2k(0x40e))));
    if(ebda){
        struct rsdp* ret = searchrsdp(ebda, ebda + 0x400);
        if(ret){
            return ret;
        }
    }
    return searchrsdp((char*)p2k(0xe0000), (char*)p2k(0x100000));
}
//使用acpi获取cpu与ioapic信息
int64_t acpiinit(){
    //首先找到rsdp结构的位置
    struct rsdp* rsdp = findrsdp();
    struct rsdt* rsdt = (struct rsdt*)p2k(rsdp->rsdtaddress);
    struct madt* madt = nullptr;
    //获取rsdt结构体中entry的数量
    uint32_t count = (rsdt->h.length - sizeof(struct sdtheader)) / 4;
    //找到madt表
    for(uint32_t i = 0; i < count; i++){
        struct sdtheader* h = (struct sdtheader*)p2k(rsdt->entry[i]);
        if(memcmp(h->signature, "APIC", 4) == 0){
            madt = (struct madt*)h;
        }
    }
    if(!madt){
        return -1;
    }
    uint32_t lapicaddr = madt->lapicaddr;
    char* start = (char*)(madt->entry);
    char* end = (char*)(start + madt->h.length - sizeof(struct madt));
    char* p = start;
    int32_t ioapicno = 0;
    while(p < end){
        if(end - start < 2){
            break;
        }
        uint8_t length = p[1];
        if(end - start < length){
            break;
        }
        switch(p[0]){
            case 0:{
                struct madtlapic* lapic = (struct madtlapic*)p;
                if(length < sizeof(struct madtlapic)){
                    break;
                }
                if(!(lapic->flags & 1)){
                    break;
                }
                cpus[cpuno].id = cpuno;
                cpus[cpuno].apicid = lapic->apicid;
                cpuno++;
            }
            break;
            case 1:{
                struct madtioapic* madtioapic = (struct madtioapic*)p;
                if(length < sizeof(struct madtioapic)){
                    break;
                }
                ioapicid = madtioapic->ioapicid;
                ioapicno++;
                ioapic = (void*)p2k((uint64_t)madtioapic->ioapicaddr);
            }
            break;
        }
        p += length;
    }
    if(cpuno){
        issmp = true;
        lapic = (void*)p2k(lapicaddr);
        return 0;
    }else{
        return 1;
    }
}