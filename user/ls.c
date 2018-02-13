#include<lib.h>
int64_t main(int64_t argc, char** argv){
    char* dir;
    if(argc == 1){
        dir = "/";
    }else{
        dir = argv[1];
    }
    struct dircontent con;
    int64_t dirret = readdir(dir, &con);
    char buf[512];
    strncopy(buf, dir, 100);
    int64_t dirlen = strlen(dir);
    if(dirret < 0){
        printf("open error\n");
        return 1;
    }
    for(uint64_t i = 0; i < 14 * 32; i++){
        if(con.c[i][0] == 0){
            break;
        }
        strncopy(dir + dirlen, con.c[i], 100);
        struct stat s;
        stat("/", &s);
        printf("%s %d ", con.c[i], s.size);
        if(s.type == 1){
            printf("file\n");
        }else{
            printf("dir\n");
        }
    }
    return 0;
}