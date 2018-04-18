#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>

/* some constants */
#define MAXLINE    1024   /* max line size */
#define MAXARGS     128   /* max args on a command line */

int pipenumber;
char prompt[3] = "# ";// as # is the prompt character
char *argnew [20] [MAXARGS] ;/*用来存放多个变量*/

/*此处使用递归的方法来实现管道*/
void executeon(int layer)
{
    int status;
    int fd[2];
    if (layer==pipenumber)
        execvp(argnew[layer][0],argnew[layer]);
    /*调用系统函数来创建管道*/
    //如果失败输出错误信息
    if (pipe(fd) == -1)
        perror("pipe");
    //创建一个子进程
    pid_t pid = fork();
    if (pid==0)
    {
        close(fd[0]);//关闭读借口
        dup2(fd[1], STDOUT_FILENO);
        close(fd[1]);
        execvp(argnew[layer][0],argnew[layer]);
    }
    do{
        pid_t w=waitpid(pid, &status, WUNTRACED | WCONTINUED);
        close(fd[1]);
        dup2(fd[0],STDIN_FILENO);
        close(fd[0]);
    } while (!WIFEXITED(status) && !WIFSIGNALED(status));

    
    executeon(layer + 1);
}



int main()
{
    /* 输入的命令行 */
    char cmd[MAXLINE];
    char string1[MAXLINE];
    char string2[MAXLINE];
    char *args[MAXARGS];
    
    /* argnew 是对管道进行处理的字符串数组*/
    int i;
    int j;
    int j2;
    int mark=0;
    while (1)
    {
        mark=0;
        memset(cmd,0,sizeof(cmd));
        memset(args,0,sizeof(args));
        pipenumber=0;
        //printf the prompt
        printf("%s",prompt);
        fflush(stdin);
        //
        fgets(cmd,256,stdin);
        for (i = 0; cmd[i] != '\n'; i++);//找到第一个换行符
        cmd[i] = '\0';
        /* 拆解命令行 */
        args[0] = cmd;
        for (i = 0; *args[i]; i++)
            for (args[i+1] = args[i] + 1; *args[i+1]; args[i+1]++)
                if (*args[i+1] == ' ') {
                    while (*args[i+1] == ' ')//这边要考虑到会有多个空格的情况,所以使用一下while循环
                    {
                        *args[i+1] = '\0';
                        args[i+1]++;
                    }
                    break;
                }
        args[i] = NULL;
        
        memset(argnew,0,sizeof(argnew));
        /*这边统计管道数目，并对字符串再一次切分*/
        for (j=0;args[j]!=NULL;j++)
        {
            if (*args[j]=='|')
            {
                argnew[pipenumber][mark]=NULL;
                ++pipenumber;
                mark=0;
            }
            else
            {
                argnew[pipenumber][mark]=args[j];
                ++mark;
            }
            
        }
        /* 没有输入命令 */
        if (!args[0])
            continue;
        
        /* 内建命令 */
        if (strcmp(args[0], "cd") == 0) {
            if (args[1])
                chdir(args[1]);
            continue;
        }
        if (strcmp(args[0], "pwd") == 0) {
            char wd[4096];
            puts(getcwd(wd, 4096));
            continue;
        }
        if (strcmp(args[0], "exit") == 0)
            return 0;
        
        if (strcmp(args[0],"export")==0)
        {
            /*这边相当于进行一下小小的句法分析即可，关键就是找到等号前面的东西，并把前面的东西作为一个字符串存起来，再找到等号后面的东西存起来即可*/
            memset(string1,0,sizeof(string1));
            memset(string2,0,sizeof(string2));
            if (((args[1])==NULL)||(args[1][0]=='\0')) continue;//如果该字符串长度为0，那么久继续
            for(j=0;args[1][j]!='=';j++)
                string1[j]=args[1][j];
            string1[j]='\0';
            j2=0;
            for (++j,j2=0;args[1][j]!='\0';j++,j2++)
                string2[j2]=args[1][j];
            string2[j2]='\0';
            setenv(string1,string2,0);
            continue;
        }
        
        
        /* 外部命令 */
        /*这里使用递归的方法，建立管道之间的通信*/
        
        
        
        
        pid_t pid = fork();
        if (pid == 0) {
            /* 子进程 */
            
            executeon(0);
            /* execvp失败 */
            return 255;
        }
        /* 父进程 */
        
        
        
        
        wait(NULL);
        
        
    }
    
}


