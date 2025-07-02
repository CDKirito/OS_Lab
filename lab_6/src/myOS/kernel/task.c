#include "../include/task.h"
#include "../include/myPrintk.h"

#define NULL ((void *)0)

void schedule(void);
void tskDestroy(myTCB *tsk);

myTCB tcbPool[TASK_NUM];//进程池的大小设置

myTCB* idleTsk;                /* idle 任务 */
myTCB* currentTsk;             /* 当前任务 */
myTCB* firstFreeTsk;           /* 下一个空闲的 TCB */

// 空闲任务，CPU空闲时运行
// 一旦有其他任务进入就绪队列，就切换到其他任务
void tskIdleBdy(void) {
    while(1){
        schedule();
    }
}

// 空任务，用于初始化进程池
void tskEmpty(void){
}

//就绪队列结构体
typedef struct rdyQueue{
    myTCB * head;       //就绪队列的头结点
    myTCB * tail;       //就绪队列的尾结点
    myTCB * idleTsk;    //空闲任务
} rdyQueue;

rdyQueue rdy_queue;

// 初始化就绪队列的头结点和尾结点，并设置空闲任务
void rdyQueueInit(myTCB* idleTsk) {
    rdy_queue.head = NULL;
    rdy_queue.tail = NULL;
    rdy_queue.idleTsk = idleTsk;     //设置空闲任务
}

// 判断就绪队列是否为空
int rdyQueueIsEmpty(void) {     //当head和tail均为NULL时，rdy_queue为空
    return (rdy_queue.head == NULL && rdy_queue.tail == NULL);
}

// 将tsk入队到就绪队列rdy_queue中
void tskEnqueue(myTCB *tsk) {
    if (rdyQueueIsEmpty()) {
        rdy_queue.head = tsk; //如果就绪队列为空，将头结点指向当前任务
        rdy_queue.tail = tsk; //尾结点也指向当前任务
    } else {
        rdy_queue.tail->nextTCB = tsk;  //将当前任务加入到尾结点的后面
        rdy_queue.tail = tsk;           //更新尾结点为当前任务
    }
    tsk->nextTCB = NULL;        //将当前任务的下一个TCB指针置为NULL
}

// 将task从就绪队列rdy_queue中出队
void tskDequeue(myTCB *tsk) {
    if (rdyQueueIsEmpty()) {   //如果就绪队列为空，直接返回
        return; 
    }
    if (rdy_queue.head == tsk) { //如果要出队的任务是头结点
        rdy_queue.head = tsk->nextTCB; //将头结点指向下一个任务
        if (rdy_queue.head == NULL) { //如果头结点变为NULL，说明队列为空
            rdy_queue.tail = NULL; //尾结点也置为NULL
        }
    } else { //如果要出队的任务不是头结点
        myTCB *prev = rdy_queue.head; //从头结点开始查找前驱节点
        while (prev->nextTCB != tsk && prev->nextTCB != NULL) {
            prev = prev->nextTCB; //找到前驱节点
        }
        if (prev->nextTCB == tsk) { //如果找到了要出队的任务
            prev->nextTCB = tsk->nextTCB; //将前驱节点的下一个指针指向要出队任务的下一个任务
            if (tsk == rdy_queue.tail) { //如果要出队的任务是尾结点
                rdy_queue.tail = prev; //更新尾结点为前驱节点
            }
        }
    }
    tsk->nextTCB = NULL;            //将出队的任务的下一个指针置为NULL
}

void tskEnd(void);

//初始化栈空间（不需要填写）
void stack_init(unsigned long **stk, void (*task)(void)){
    // 硬件保存上下文，由 iret 指令恢复
    *(*stk)-- = (unsigned long) tskEnd;
    *(*stk)-- = (unsigned long) task;       //EIP
    *(*stk)-- = (unsigned long) 0x0202;     //FLAG寄存器

    *(*stk)-- = (unsigned long) 0xAAAAAAAA; //EAX
    *(*stk)-- = (unsigned long) 0xCCCCCCCC; //ECX
    *(*stk)-- = (unsigned long) 0xDDDDDDDD; //EDX
    *(*stk)-- = (unsigned long) 0xBBBBBBBB; //EBX

    *(*stk)-- = (unsigned long) 0x44444444; //ESP，出栈时跳过
    *(*stk)-- = (unsigned long) 0x55555555; //EBP
    *(*stk)-- = (unsigned long) 0x66666666; //ESI
    *(*stk)   = (unsigned long) 0x77777777; //EDI
}

// 任务开始：任务状态设置为就绪 + 任务入队
void tskStart(myTCB *tsk){
    tsk->TSK_State = TSK_RDY;
    tskEnqueue(tsk);
}

// 结束任务：任务出队 + 销毁任务 + 调度下一个任务
void tskEnd(void){
    tskDequeue(currentTsk);
    tskDestroy(currentTsk);
    schedule();
}

// 创建任务：选择一个空闲的TCB，设置任务入口地址和栈空间，然后将任务加入到就绪队列中，返回任务ID
int tskCreate(void (*tskBody)(void)){
    if (firstFreeTsk == NULL) {        //没有空闲的TCB可用
        myPrintk(0x2, "No free TCB available.\n");
        return -1; 
    }
    
    myTCB *newTsk = firstFreeTsk;      //获取下一个空闲的TCB
    firstFreeTsk = newTsk->nextTCB;    //更新下一个空闲的TCB

    newTsk->task_entrance = tskBody;   //设置任务入口地址
    stack_init(&(newTsk->stkTop), tskBody); //初始化栈空间，栈顶指针已在TaskManagerInit初始化

    tskStart(newTsk);  //将任务状态设置为就绪，并将任务入队
    
    return newTsk->TSK_ID; //返回新创建任务的ID
}

// 销毁进程：设置任务状态为未分配，将TCB加入到空闲链表中
void tskDestroy(myTCB *tsk) {
    int takIndex = tsk->TSK_ID; //获取任务ID
    if (takIndex < 0 || takIndex >= TASK_NUM) {
        myPrintk(0x2, "Invalid task index: %d\n", takIndex);
        return; //无效的任务索引
    }
    
    myTCB *tsk = &tcbPool[takIndex]; //获取对应的TCB
    if (tsk->TSK_State == TSK_NONE) {
        myPrintk(0x2, "Task %d is not allocated.\n", takIndex);
        return; //任务未分配，无法销毁
    }

    tsk->TSK_State = TSK_NONE;    //将状态设置为未分配
    tsk->nextTCB = firstFreeTsk;  //将该TCB加入到空闲链表中
    firstFreeTsk = tsk;           //更新下一个空闲的TCB指针
}

unsigned long **prevTSK_StackPtr;
unsigned long *nextTSK_StackPtr;

// 切换上下文
void context_switch(myTCB *prevTsk, myTCB *nextTsk) {
     prevTSK_StackPtr = &(prevTsk->stkTop);
     currentTsk = nextTsk;
     nextTSK_StackPtr = nextTsk->stkTop;
     CTX_SW(prevTSK_StackPtr,nextTSK_StackPtr);
}

// 获取下一个任务（FCFS调度）
myTCB * nextTskFCFS(void) {
    if (rdyQueueIsEmpty()) {
        return rdy_queue.idleTsk;   //如果就绪队列为空，返回空闲任务
    }
    return rdy_queue.head;          //否则返回头结点
}

// FCFS调度
void scheduleFCFS(void) {
    myTCB *nextTsk;
    nextTsk = nextTskFCFS();
    if (currentTsk->have_run_time >= currentTsk->run_time) {
        context_switch(currentTsk,nextTsk);
    }
}

// 获取下一个任务（SJF调度）
myTCB * nextTskSJF(void) {
    if (rdyQueueIsEmpty()) {
        return rdy_queue.idleTsk;   //如果就绪队列为空，返回空闲任务
    }
    myTCB *shortestJob = rdy_queue.head;
    myTCB *current = rdy_queue.head->nextTCB;
    while (current != NULL) {
        if (current->run_time < shortestJob->run_time) {
            shortestJob = current;
        }
        current = current->nextTCB;
    }
    return shortestJob;
}

// SJF调度
void scheduleSJF(void) {
    myTCB *nextTsk;
    nextTsk = nextTskSJF();
    if (currentTsk->have_run_time >= currentTsk->run_time) {
        context_switch(currentTsk,nextTsk);
    }
}

// 获取下一个任务（RR调度）
myTCB * nextTskRR(void) {
    if (rdyQueueIsEmpty()) {
        return rdy_queue.idleTsk;   //如果就绪队列为空，返回空闲任务
    }
    return rdy_queue.head;          //否则返回头结点
}

// RR调度
void scheduleRR(void) {
    myTCB *nextTsk;
    nextTsk = nextTskRR();
    if (timeSlice <= 0) {
        context_switch(currentTsk,nextTsk);
        timeSlice = TIME_SLICE; // 重置时间片
    } else if (currentTsk->have_run_time >= currentTsk->run_time) {
        context_switch(currentTsk,nextTsk);
        timeSlice = TIME_SLICE; // 重置时间片
    } else {
        timeSlice--; // 减少时间片
    }
}

//调度算法
void schedule(void) {
     scheduleFCFS();
}

// 进入多任务调度模式
unsigned long BspContextBase[STACK_SIZE];              // BspContextBase 是一块为“主控线程/主程序”（即OS启动时的主流程）分配的专用栈空间。
unsigned long *BspContext;
void startMultitask(void) {
     BspContext = BspContextBase + STACK_SIZE -1;
     prevTSK_StackPtr = &BspContext;                   // 伪装“当前任务”的栈顶指针为主控流程的栈顶
     currentTsk = nextFCFSTsk();                       // 选出下一个要运行的任务（通常是 init 任务或 idle 任务）
     nextTSK_StackPtr = currentTsk->stkTop;
     CTX_SW(prevTSK_StackPtr,nextTSK_StackPtr);        // 切换到下一个任务的上下文
}

// 任务管理器初始化：初始化进程池 + 创建 idle 任务 + 创建初始化任务 + 进入多任务模式
void TaskManagerInit(void) {
     // 初始化进程池（所有的进程状态都是TSK_NONE）
     myTCB * thisTCB;
     for(int i = 0; i < TASK_NUM; i++){                     //对进程池tcbPool中的进程进行初始化处理
          thisTCB = &tcbPool[i];
          thisTCB->TSK_ID = i;
          thisTCB->stkTop = thisTCB->stack+STACK_SIZE-1;    //将栈顶指针复位
          thisTCB->TSK_State = TSK_NONE;                    //表示该进程池未分配，可用
          thisTCB->task_entrance = tskEmpty;                //将任务入口地址设置为tskEmpty
          if(i==TASK_NUM-1){
               thisTCB->nextTCB = (void *)0;
          }
          else{
               thisTCB->nextTCB = &tcbPool[i+1];
          }
     }
     //创建idle任务
     idleTsk = &tcbPool[0];                       //将tcbPool[0]作为idle任务
     stack_init(&(idleTsk->stkTop),tskIdleBdy);
     idleTsk->task_entrance = tskIdleBdy;
     idleTsk->nextTCB = (void *)0;
     idleTsk->TSK_State = TSK_RDY;
     rqFCFSInit(idleTsk);

     firstFreeTsk = &tcbPool[1];
     
     //创建init任务
     tskCreate(initTskBody);

     //进入多任务状态
     myPrintk(0x2,"START MULTITASKING......\n");
     startMultitask();
     myPrintk(0x2,"STOP MULTITASKING......SHUT DOWN\n");

}
