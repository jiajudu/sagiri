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
    struct stat s;
    int64_t statret = stat("/", &s);
    printf("statret = %d, type = %d, size = %d\n", statret, s.type, s.size);
    statret = stat("/license", &s);
    printf("statret = %d, type = %d, size = %d\n", statret, s.type, s.size);
    statret = stat("/hello", &s);
    printf("statret = %d, type = %d, size = %d\n", statret, s.type, s.size);
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