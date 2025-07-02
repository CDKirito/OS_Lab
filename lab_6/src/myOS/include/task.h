#ifndef __TASK_H__
#define __TASK_H__

#ifndef USER_TASK_NUM
#include "../../userApp/userApp.h"
#endif

#define TASK_NUM (2 + USER_TASK_NUM)    // 任务数量：空闲 + 初始化 + 用户任务
#define initTskBody myMain              // 初始化任务宏定义
#define STACK_SIZE 512                  // 任务栈空间大小
#define TIME_SLICE 10                   // 时间片大小

// 进程状态
#define TSK_RDY     0        //表示当前进程已经进入就绪队列中
#define TSK_WAIT    1        //表示当前进程还未进入就绪队列中
#define TSK_RUNNING  2        //表示当前进程正在运行
#define TSK_NONE    3        //表示进程池中的TCB为空未进行分配

void initTskBody(void);

void CTX_SW(void*prev_stkTop, void*next_stkTop);

// 进程结构体
typedef struct myTCB {
     unsigned long *stkTop;             /* 栈顶指针 */
     unsigned long stack[STACK_SIZE];   /* 开辟了一个大小为STACK_SIZE的栈空间 */  
     unsigned long TSK_State;           /* 进程状态 */
     unsigned long TSK_ID;              /* 进程ID */ 
     void (*task_entrance)(void);       /*进程的入口地址*/
     int arrival_time;                  /* 进程的到达时间 */
     int run_time;                      /* 进程的运行时间 */
     int have_run_time;                 /* 进程已经运行的时间 */
     struct myTCB * nextTCB;            /*下一个TCB*/
} myTCB;

extern int timeSlice;                // 时间片大小

extern myTCB tcbPool[TASK_NUM];         // 进程池的大小设置

extern myTCB* idleTsk;                  // idle 任务
extern myTCB* currentTsk;               // 当前任务
extern myTCB* firstFreeTsk;             // 下一个空闲的 TCB


void TaskManagerInit(void);

#endif
