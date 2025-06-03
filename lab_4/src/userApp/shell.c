//shell.c --- malloc version
#include "../myOS/userInterface.h"

#define NULL (void*)0

// 从串口读取一行命令，存到缓冲区，遇到回车结束输入，并自动回显。
int getCmdline(unsigned char *buf, int limit){
    unsigned char *ptr = buf;
    int n = 0;
    while (n < limit) {
        *ptr = uart_get_char();        
        if (*ptr == 0xd) {  // 回车
            *ptr++ = '\n';  // 把换行符 \n 写入当前缓冲区，然后指针后移
            *ptr = '\0';
            uart_put_char('\r');
            uart_put_char('\n');
            return n+2;
        }
        uart_put_char(*ptr);
        ptr++;
        n++;
    }     
    return n;
}

typedef struct cmd {
    unsigned char cmd[20+1];                        // 命令名，最大20字节
    int (*func)(int argc, unsigned char **argv);    // 命令对应的函数指针
    void (*help_func)(void);                        // 帮助函数指针
    unsigned char description[100+1];               // 命令描述，最大100字节
    struct cmd * nextCmd;              // 指向下一个命令的指针，用于链表结构
} cmd;

#define cmd_size sizeof(struct cmd)

struct cmd *ourCmds = NULL;

int listCmds(int argc, unsigned char **argv){
    struct cmd *tmpCmd = ourCmds;
    myPrintf(0x7, "list all registered commands:\n");
    myPrintf(0x7, "command name: description\n");

    while (tmpCmd != NULL) {
        myPrintf(0x7,"% 12s: %s\n",tmpCmd->cmd, tmpCmd->description);
        tmpCmd = tmpCmd->nextCmd;
    }
    return 0;
}

void addNewCmd(	unsigned char *cmd, 
		int (*func)(int argc, unsigned char **argv), 
		void (*help_func)(void), 
		unsigned char* description){
	// TODO
    /*功能：增加命令
    1. 使用malloc创建一个cmd的结构体，新增命令。
    2. 同时还需要维护一个表头为ourCmds的链表。
    */
    struct cmd *newCmd = (struct cmd *)malloc(cmd_size);
    if (newCmd == NULL) {
        myPrintf(0x7,"addNewCmd: malloc failed\n");
        return;
    }

    // 初始化新命令
    char *ptr = newCmd->cmd;
    while (*cmd) *ptr++ = *cmd++;   // 将命令字符串复制到 cmd 字段
    *ptr = '\0';
    newCmd->func = func;            // 设置命令对应的函数指针
    newCmd->help_func = help_func;  // 设置帮助函数指针
    char *descPtr = newCmd->description;
    while (*description) *descPtr++ = *description++;   // 将描述字符串复制到 description 字段
    *descPtr = '\0';
    newCmd->nextCmd = NULL;         // 初始化下一个命令指针为 NULL

    // 将新命令添加到链表的末尾
    if (ourCmds == NULL) {
        ourCmds = newCmd;  // 如果链表为空，直接设置为新命令
    } else {
        struct cmd *tmpCmd = ourCmds;
        while (tmpCmd->nextCmd != NULL) {
            tmpCmd = tmpCmd->nextCmd;  // 找到链表的最后一个节点
        }
        tmpCmd->nextCmd = newCmd;  // 将新命令添加到链表末尾
    }
}

void help_help(void){
    myPrintf(0x7,"USAGE: help [cmd]\n\n");
}

int help(int argc, unsigned char **argv){
    int i;
    struct cmd *tmpCmd = ourCmds;
    help_help();

    // 如果只输入了 help（即 argc==1），调用 listCmds，列出所有命令和简介
    if (argc==1) return listCmds(argc,argv);
    // argc > 2, 显示错误信息
    if (argc>2) return 1;
    
    // argc == 2, 显示该命令的帮助信息
    while (tmpCmd != NULL) {            
        if (strcmp(argv[1],tmpCmd->cmd)==0) {
            if (tmpCmd->help_func!=NULL)
                tmpCmd->help_func();
            else myPrintf(0x7,"%s\n",tmpCmd->description);
            break;
        }
        tmpCmd = tmpCmd->nextCmd;
    }
    return 0;
}

struct cmd *findCmd(unsigned char *cmd){
    struct cmd * tmpCmd = ourCmds;
	int found = 0;
    while (tmpCmd != NULL) {          
        if (strcmp(cmd,tmpCmd->cmd)==0){
            found=1;
            break;
        }
        tmpCmd = tmpCmd->nextCmd;
    }
	return found?tmpCmd:NULL;
}

// 将命令行字符串分割成单词，存入 argv 数组中，返回单词数
int split2Words(unsigned char *cmdline, unsigned char **argv, int limit){
    unsigned char c, *ptr = cmdline;
    int argc=0;    
    int inAWord=0;

    while ( c = *ptr ) {  // not '\0'
        if (argc >= limit) {
            myPrintf(0x7,"cmdline is tooooo long\n");
            break;
        }
        switch (c) {
            case ' ':  *ptr = '\0'; inAWord = 0; break; //skip white space
            case '\n': *ptr = '\0'; inAWord = 0; break; //end of cmdline?
            default:  //a word
             if (!inAWord) *(argv + argc++) = ptr;
             inAWord = 1;             
             break;
        }   
        ptr++;     
    }
    return argc;
}

void initShell(void){
    addNewCmd("cmd\0",listCmds,NULL,"list all registered commands\0");
    addNewCmd("help\0",help,help_help,"help [cmd]\0");
}

unsigned char cmdline[100];
void startShell(void){    
    unsigned char *argv[10];  //max 10
    int argc;    
    struct cmd *tmpCmd;
    //myPrintf(0x7,"StartShell:\n");     
    
    while(1) {
        myPrintf(0x3,"Student >:");
        getCmdline(&cmdline[0],100);
        myPrintf(0x7,cmdline);

        argc = split2Words(cmdline,&argv[0],10); 
        if (argc == 0) continue;

	    tmpCmd = findCmd(argv[0]);
        if (tmpCmd)   
	        tmpCmd->func(argc, argv);
	    else
            myPrintf(0x7,"UNKNOWN command: %s\n",argv[0]);
    }
}
