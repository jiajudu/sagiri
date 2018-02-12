#include<lib.h>
int64_t main(){
    printf("Hello World!\n");
    struct dircontent con;
    int64_t dirret = readdir("/", &con);
    printf("dirret = %d\n", dirret);
    for(uint64_t i = 0; i < 14 * 32; i++){
        if(con.c[i][0] == 0){
            break;
        }
        printf("in /: %s\n", con.c[i]);
    }
    int64_t fd = open("/license", 1);
    char buf[100];
    int64_t readret = 100;
    while(readret == 100){
        readret = read(fd, buf, 100);
        for(uint64_t i = 0; i < readret; i++){
            put(buf[i]);
        }
    }
    close(fd);
    int64_t unlinkret = unlink("/license");
    printf("unlinkret = %d\n", unlinkret);
    fd = open("/license", 1);
    printf("fd = %d\n", fd);
    dirret = readdir("/", &con);
    printf("dirret = %d\n", dirret);
    for(uint64_t i = 0; i < 14 * 32; i++){
        if(con.c[i][0] == 0){
            break;
        }
        printf("in /: %s\n", con.c[i]);
    }
    return 0;
}