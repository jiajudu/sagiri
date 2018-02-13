#include<lib.h>
int64_t readline(char* buf, int64_t length){
    int64_t ret = 0;
    for(uint64_t i = 0; i < length; i++){
        int64_t readret = 0;
        while(readret == 0){
            readret = read(0, buf + i, 1);
        }
        ret++;
        if(buf[i] == 13){
            buf[i] = '\n';
        }
        //printf("%d\n", buf[i]);
        write(1, buf + i, 1);
        if(buf[i] == '\n'){
            break;
        }
    }
    return ret;
}
int64_t main(int64_t argc, char** argv){
    printf("this is sh.\n");
    while(true){
        printf("# ");
        char buf[100];
        for(uint64_t i = 0; i < 100; i++){
            buf[i] = 0;
        }
        int64_t ret = readline(buf, 100);
        buf[ret - 1] = 0;
        int64_t length = strlen(buf);
        uint64_t args[10];
        char last = ' ';
        uint64_t ptr = 0;
        for(uint64_t i = 0; i < length; i++){
            if(last == ' ' && buf[i] != ' '){
                args[ptr] = (uint64_t)&(buf[i]);
                ptr++;
                if(ptr == 10){
                    break;
                }
            }
            last = buf[i];
        }
        for(uint64_t i = 0; i < length; i++){
            if(buf[i] == ' '){
                buf[i] = 0;
            }
        }
        if(ptr == 0){
            continue;
        }
        args[ptr] = 0;
        int64_t pid = fork();
        if(pid == 0){
            exec((char*)args[0], args);
            printf("exec failed. try /echo hello world\n");
            exit(1);
        }else{
            int64_t ret;
            waitproc(pid, &ret);
        }
    }
    return 0;
}