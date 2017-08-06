#include <stdio.h>
#include <stdlib.h>
int main(){
    FILE* f = fopen("out/bootblock", "rb");
    fseek(f, 0, 2);
    long size = ftell(f);
    if(size > 512){
        printf("The bootblock is too large!");
        exit(1);
    }
    fseek(f, 0, 0);
    char buf[512];
    fread(buf, 1, 512, f);
    fclose(f);
    buf[510] = 0x55;
    buf[511] = 0xaa;
    f = fopen("out/bootblock", "wb");
    fwrite(buf, 1, 512, f);
    fclose(f);
    return 0;
}