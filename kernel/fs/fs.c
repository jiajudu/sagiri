#include<fs/fs.h>
#include<lib/stdio.h>
#include<driver/ide.h>
struct superblock sb;
struct superblock readsuperblock(){
    char buf[512];
    readsect((uint64_t)buf, 0);
    struct superblock sb = *((struct superblock *)buf);
    return sb;
}
struct inode getinode(uint64_t inodenum){
    uint64_t no = inodenum / 8 + 1 + sb.bitmapblock;
    char buf[512];
    readsect((uint64_t)buf, no);
    struct inode ret = *(((struct inode*)buf) + inodenum % 8);
    return ret;
}
void getpage(uint64_t pagenum, char* buf){
    uint64_t no = pagenum + 1 + sb.bitmapblock + sb.inodeblocks;
    readsect((uint64_t)buf, no);
}
void printfile(struct inode* n){
    if(n->type == 1){
        uint64_t childnum = n->size / 16;
        printf("directory, childnum = %d\n", childnum);
        uint64_t blockno = (childnum + 31) / 32;
        uint64_t currentchild = 0;
        for(uint64_t i = 0; i < blockno; i++){
            struct dirent ds[32];
            getpage(n->addr[i], (char*)ds);
            for(uint64_t c = 0; c < 32; c++){
                printf("file, inode = %d, name = %s\n", ds[c].inodenum, ds[c].name);
                struct inode childinode = getinode(ds[c].inodenum);
                printfile(&childinode);
                currentchild++;
                if(currentchild >= childnum){
                    break;
                }
            }
        }
    }else{
        printf("file, size = %d\n", n->size);
    }
}
void fsinit(){
    printf("This is fs init.\n");
    sb = readsuperblock();
    printf("size = %d, bitmap block = %d, inodeblock = %d, datablock = %d, rootinode = %d\n", sb.size, sb.bitmapblock, sb.inodeblocks, sb.datablocks, sb.rootinode);
    struct inode rootinode = getinode(sb.rootinode);
    printfile(&rootinode);
}