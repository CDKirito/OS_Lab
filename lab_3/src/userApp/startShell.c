#include "io.h"
#include "myPrintk.h"
#include "uart.h"
#include "vga.h"
#include "i8253.h"
#include "i8259A.h"
#include "tick.h"
#include "wallClock.h"

#define EXIT -1
#define MAX_CMD 10
#define NULL ((void *)0)

typedef struct Command {
    char name[80];
    char help_content[200];
    int (*func)(int argc, char (*argv)[8]);
} myCommand;

// 命令函数声明
int func_cmd(int argc, char (*argv)[8]);
int func_help(int argc, char (*argv)[8]);
int func_exit(int argc, char (*argv)[8]);

// 命令表
myCommand myCmds[MAX_CMD] = {
    {"cmd", "List all command\n", func_cmd},
    {"help", "Show help for commands\n", func_help},
    {"exit", "Exit the shell\n", func_exit},
    {"\0", "\0", NULL} // 结束标志
};

// 字符串比较函数
int strcmp(const char *s1, const char *s2) {
    while (*s1 && *s2 && (*s1 == *s2)) {
        s1++;
        s2++;
    }
    return *(unsigned char *)s1 - *(unsigned char *)s2;
}

// 查找命令函数
int findCommand(char *name, myCommand *cmds, int cmd_count) {
    for (int i = 0; i < cmd_count; i++) {
        if (cmds[i].func == NULL) break; // 到达结尾
        if (strcmp(name, cmds[i].name) == 0) {
            return i;
        }
    }
    return -1; // 未找到
}

// cmd命令：列出所有命令
int func_cmd(int argc, char (*argv)[8]) {
    myPrintk(0x07, "Command list:\n\0");
    for (int i = 0; myCmds[i].func != NULL; i++) {
        myPrintk(0x07, myCmds[i].name);
        myPrintk(0x07, "\n\0");
    }
    return 0;
}

// help命令：显示帮助
int func_help(int argc, char (*argv)[8]) {
    if (argc == 1) {
        myPrintk(0x07, "Usage: help [command]\n\0");
        return 0;
    }
    for (int i = 0; myCmds[i].func != NULL; i++) {
        if (strcmp(argv[1], myCmds[i].name) == 0) {
            myPrintk(0x07, myCmds[i].help_content);
            return 0;
        }
    }
    myPrintk(0x07, "No such command\n\0");
    return 0;
}

// exit命令：退出shell
int func_exit(int argc, char (*argv)[8]) {
    myPrintk(0x07, "Bye!\n\0");
    return EXIT;
}

// 解析输入为argc和argv
void parseInput(char *buf, int *argc, char argv[8][8]) {
    *argc = 0;
    int i = 0, j = 0, k = 0;
    while (buf[i] != '\0' && *argc < 8) {
        // 跳过空格
        while (buf[i] == ' ' || buf[i] == '\t') i++;
        if (buf[i] == '\0') break;
        j = 0;
        while (buf[i] != ' ' && buf[i] != '\t' && buf[i] != '\0' && j < 7) {
            argv[*argc][j++] = buf[i++];
        }
        argv[*argc][j] = '\0';
        (*argc)++;
    }
}

void startShell(void) {
    char BUF[256]; //输入缓存区
    int BUF_len = 0;	//输入缓存区的长度
    int argc;
    char argv[8][8];

    do {
        BUF_len = 0;
        myPrintk(0x07, "Student>>\0");
        while ((BUF[BUF_len] = uart_get_char()) != '\r') {
            uart_put_char(BUF[BUF_len]);
            BUF_len++;
        }
        uart_put_chars(" -pseudo_terminal\0");
        uart_put_char('\n');
        BUF[BUF_len] = '\0'; // 
        
        myPrintk(0x07, BUF);
        myPrintk(0x07, "\n\0");

        // 解析输入
        parseInput(BUF, &argc, argv);

        if (argc == 0) continue;

        int idx = findCommand(argv[0], myCmds, MAX_CMD);
        if (idx >= 0) {
            int ret = myCmds[idx].func(argc, argv);
            if (ret == EXIT) break;
        } else {
            myPrintk(0x07, "Unknown command\n\0");
        }
    } while (1);
}

