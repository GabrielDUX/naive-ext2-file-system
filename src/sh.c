#include "util.h"
#include "sh.h"
#include "filesys.h"
#include "disk.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>


char whitespace[] = " \t\r\n\v";
char args[MAXARGS][MAXWORD];

int getcmd(char *buf, int nbuf){
    printf("=> ");
    memset(buf,0,nbuf);
    gets(buf,nbuf);

    if(buf[0]==0) //EOF
        return -1;
    return 0;
}

void getargs(char *cmd,char *argv[],int *argc){
    // argv存放各个参数的指针
    for(int i=0;i<MAXARGS;i++){
        argv[i] = &args[i][0];
    }

    int i = 0; // 表示第i个word
    int j = 0;
    for (; cmd[j] != '\n' && cmd[j] != '\0'; j++)
    {
        // 每一轮循环都是找到输入的命令中的一个word，比如 echo hi ,就是先找到echo，再找到hi
        // 让argv[i]分别指向他们的开头，并且将echo，hi后面的空格设为\0
        // 跳过之前的空格
        while (strchr(whitespace,cmd[j])){
            j++;
        }
        argv[i++]=cmd+j;
        // 只要不是空格，就j++,找到下一个空格
        while (strchr(whitespace,cmd[j])==0){
            j++;
        }
        cmd[j]='\0';
    }

    argv[i]=0;
    *argc=i;
}

int runcmd(char *argv[],int argc){
    // 测试是否成功获取到了cmd
    // for(int i=0;i<argc;i++){
    //     printf("%d:%s ",i,argv[i]);
    // }
    // printf("\n");
    
    if(!strcmp(argv[0],"ls")){
        exec_ls(argv,argc);
    }
    else if(!strcmp(argv[0],"mkdir")){
        exec_mkdir(argv,argc);
    }
    else if(!strcmp(argv[0],"touch")){
        exec_touch(argv,argc);
    }
    else if(!strcmp(argv[0],"cp")){
        exec_cp(argv,argc);
    }
    else if(!strcmp(argv[0],"shutdown")){
        shutdown_filesys();
    } else {
        printf("can not parse command.\n");
        return -1;
    }
    return 0;
}






void run_shell(){

    printf("booting shell... \n");
    printf("shell booted!\n");

    init_filesystem();

    char buf[MAXLINE];

    //  持续读命令
    while(getcmd(buf,MAXLINE) >= 0) {
        char* argv[MAXARGS];
        int argc = -1;
        getargs(buf,argv,&argc);
        runcmd(argv,argc);
    }

    shutdown_filesys();
}