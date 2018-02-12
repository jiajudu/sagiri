#include<stdio.h>
#include<stdint.h>
#include<sys/stat.h>
#include<string.h>
struct superblock{
    uint64_t size;
    uint64_t bitmapblock;
    uint64_t inodeblocks;
    uint64_t datablocks;
    uint64_t rootinode;
};
struct inode{
    uint32_t type;
    uint32_t size;
    uint32_t addr[14];
};
struct dirent{
    uint32_t inodenum;
    char name[12];
};
char data[5120000];
int currentinode = 0;
int currentdata = 0;
void makeused(int index){
    char* start = data + 512;
    start[index / 8] |= (1 << (index % 8));
}
struct inode* getinode(){
    struct inode* ret = (struct inode*)(data + 4 * 512 + 64 * currentinode);
    currentinode++;
    if(currentinode % 8 == 0){
        makeused(3 + currentinode / 8);
    }
    return ret;
}
char* getdatablock(){
    char* ret = data + 1004 * 512 + currentdata * 512;
    makeused(1004 + currentdata);
    currentdata++;
    return ret;
}
void writesuperblock(){
    struct superblock* sb = (struct superblock*)data;
    sb->size = 10000;
    sb->bitmapblock = 3;
    sb->inodeblocks = 1000;
    sb->datablocks = 8996;
    sb->rootinode = 0;
    makeused(0);
}
struct inode* addfile(char* name){
    struct inode* finode = getinode();
    finode->type = 1;
    struct stat statbuf;
    stat(name, &statbuf);
    finode->size = statbuf.st_size;
    int c = (finode->size + 511) / 512;
    FILE* f = fopen(name, "rb");
    if(c <= 13){
        for(int i = 0; i < c; i++){
            char* buf = getdatablock();
            fread(buf, 1, 512, f);
            int datablockno = (buf - data - 1004 * 512) / 512;
            finode->addr[i] = datablockno;
        }
    }else{
        for(int i = 0; i < 13; i++){
            char* buf = getdatablock();
            fread(buf, 1, 512, f);
            int datablockno = (buf - data - 1004 * 512) / 512;
            finode->addr[i] = datablockno;
        }
        char* extra = getdatablock();
        int datablockno = (extra - data - 1004 * 512) / 512;
        finode->addr[13] = datablockno;
        for(int i = 13; i < c; i++){
            char* buf = getdatablock();
            fread(buf, 1, 512, f);
            int datablockno = (buf - data - 1004 * 512) / 512;
            *(((int*)extra) + (i - 13)) = datablockno;
        }
    }
    fclose(f);
    return finode;
}
void adddir(){
    struct inode* dinode = getinode();
    dinode->type = 2;
    dinode->size = 0;
    char* content = getdatablock();
    int datablockno = (content - data - 1004 * 512) / 512;
    dinode->addr[0] = datablockno;
    struct inode* filenode = addfile("uobj/hello.exe");
    struct dirent* d = (struct dirent*)(content + 16 * 0);
    strcpy(d->name, "hello");
    d->name[5] = 0;
    d->inodenum = ((char*)filenode - data - 4 * 512) / 64;
    dinode->size += 16;
    filenode = addfile("license.txt");
    d = (struct dirent*)(content + 16 * 1);
    strcpy(d->name, "license");
    d->name[7] = 0;
    d->inodenum = ((char*)filenode - data - 4 * 512) / 64;
    dinode->size += 16;
    filenode = addfile("uobj/echo.exe");
    d = (struct dirent*)(content + 16 * 2);
    strcpy(d->name, "echo");
    d->name[4] = 0;
    d->inodenum = ((char*)filenode - data - 4 * 512) / 64;
    dinode->size += 16;
}
int main(){
    writesuperblock();
    makeused(1);
    makeused(2);
    makeused(3);
    adddir();
    FILE* f = fopen("fs.img", "wb");
    fwrite(data, 1, 5120000, f);
    fclose(f);
    return 0;
}