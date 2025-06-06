#include "../include/task.h"
#include "../include/myPrintk.h"

#define NULL ((void *)0)

void schedule(void);
void destroyTsk(int takIndex);

unsigned long pMemHandler;

myTCB tcbPool[TASK_NUM];//进程池的大小设置

myTCB* idleTsk;                /* idle 任务 */
myTCB* currentTsk;             /* 当前任务 */
myTCB* firstFreeTsk;           /* 下一个空闲的 TCB */

#define TSK_RDY 0        //表示当前进程已经进入就绪队列中
#define TSK_WAIT -1      //表示当前进程还未进入就绪队列中
#define TSK_RUNING 1     //表示当前进程正在运行
#define TSK_NONE 2       //表示进程池中的TCB为空未进行分配

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

//就绪队列的结构体
typedef struct rdyQueueFCFS{
     myTCB * head;       //就绪队列的头结点
     myTCB * tail;       //就绪队列的尾结点
     myTCB * idleTsk;    //空闲任务
} rdyQueueFCFS;

rdyQueueFCFS rqFCFS;

//TODO:初始化就绪队列
// FCFS 算法需要就绪队列实现先进先出
void rqFCFSInit(myTCB* idleTsk) {  //对rqFCFS进行初始化处理
     rqFCFS.head = NULL;           //初始化头结点
     rqFCFS.tail = NULL;           //初始化尾结点
     rqFCFS.idleTsk = idleTsk;     //设置空闲任务
}

//TODO: 如果就绪队列为空，返回True（需要填写）
int rqFCFSIsEmpty(void) {//当head和tail均为NULL时，rqFCFS为空
     return (rqFCFS.head == NULL && rqFCFS.tail == NULL);
}

//TODO: 从就绪队列rqFCFS中获取下一个任务，如果为空，返回空闲任务
myTCB * nextFCFSTsk(void) {//获取下一个Tsk
     if (rqFCFSIsEmpty()) {
          return rqFCFS.idleTsk; //如果就绪队列为空，返回空闲任务
     }
     return rqFCFS.head; //否则返回头结点
}

//TODO:将tsk入队到就绪队列rqFCFS中
void tskEnqueueFCFS(myTCB *tsk) {//将tsk入队rqFCFS
     if (rqFCFSIsEmpty()) {
          rqFCFS.head = tsk; //如果就绪队列为空，将头结点指向当前任务
          rqFCFS.tail = tsk; //尾结点也指向当前任务
     } else {
          rqFCFS.tail->nextTCB = tsk; //将当前任务加入到尾结点的后面
          rqFCFS.tail = tsk; //更新尾结点为当前任务
     }
     tsk->nextTCB = NULL; //将当前任务的下一个TCB指针置为NULL
}

//TODO:将task从就绪队列rqFCFS中出队，并设置状态为TSK_WAIT
void tskDequeueFCFS(myTCB *tsk) {//rqFCFS出队
     if (rqFCFSIsEmpty()) {
          return; //如果就绪队列为空，直接返回
     }
     if (rqFCFS.head == tsk) { //如果要出队的任务是头结点
          rqFCFS.head = tsk->nextTCB; //将头结点指向下一个任务
          if (rqFCFS.head == NULL) { //如果头结点变为NULL，说明队列为空
               rqFCFS.tail = NULL; //尾结点也置为NULL
          }
     } else { //如果要出队的任务不是头结点
          myTCB *prev = rqFCFS.head; //从头结点开始查找前驱节点
          while (prev->nextTCB != tsk && prev->nextTCB != NULL) {
               prev = prev->nextTCB; //找到前驱节点
          }
          if (prev->nextTCB == tsk) { //如果找到了要出队的任务
               prev->nextTCB = tsk->nextTCB; //将前驱节点的下一个指针指向要出队任务的下一个任务
               if (tsk == rqFCFS.tail) { //如果要出队的任务是尾结点
                    rqFCFS.tail = prev; //更新尾结点为前驱节点
               }
          }
     }
}

//初始化栈空间（不需要填写）
void stack_init(unsigned long **stk, void (*task)(void)){
     *(*stk)-- = (unsigned long) 0x08;       //高地址
     *(*stk)-- = (unsigned long) task;       //EIP
     *(*stk)-- = (unsigned long) 0x0202;     //FLAG寄存器

     *(*stk)-- = (unsigned long) 0xAAAAAAAA; //EAX
     *(*stk)-- = (unsigned long) 0xCCCCCCCC; //ECX
     *(*stk)-- = (unsigned long) 0xDDDDDDDD; //EDX
     *(*stk)-- = (unsigned long) 0xBBBBBBBB; //EBX

     *(*stk)-- = (unsigned long) 0x44444444; //ESP
     *(*stk)-- = (unsigned long) 0x55555555; //EBP
     *(*stk)-- = (unsigned long) 0x66666666; //ESI
     *(*stk)   = (unsigned long) 0x77777777; //EDI

}

// 将一个在进程池中的TCB设置为就绪状态，并将其加入到就绪队列中
void tskStart(myTCB *tsk){
     tsk->TSK_State = TSK_RDY;
     tskEnqueueFCFS(tsk);
}

//进程池中一个在就绪队列中的TCB的结束（不需要填写）
void tskEnd(void){
     //将一个在就绪队列中的TCB移除就绪队列
     tskDequeueFCFS(currentTsk);
     //由于TCB结束，我们将进程池中对应的TCB也删除
     destroyTsk(currentTsk->TSK_ID);
     //TCB结束后，我们需要进行一次调度
     schedule();
}

//TODO: 以tskBody为参数在进程池中创建一个进程，并调用tskStart函数，将其加入就绪队列（需要填写）
int createTsk(void (*tskBody)(void)){//在进程池中创建一个进程，并把该进程加入到rqFCFS队列中
     if (firstFreeTsk == NULL) {        //没有空闲的TCB可用
          myPrintk(0x2, "No free TCB available.\n");
          return -1; 
     }
     
     myTCB *newTsk = firstFreeTsk;      //获取下一个空闲的TCB
     firstFreeTsk = newTsk->nextTCB;    //更新下一个空闲的TCB

     newTsk->task_entrance = tskBody;   //设置任务入口地址
     stack_init(&(newTsk->stkTop), tskBody); //初始化栈空间，栈顶指针已在TaskManagerInit初始化

     tskStart(newTsk); //将新创建的任务加入到就绪队列中
     
     return newTsk->TSK_ID; //返回新创建任务的ID
}

//TODO: 销毁进程，将其状态设置为TSK_NONE，并将其加入到空闲链表中
void destroyTsk(int takIndex) {//在进程中寻找TSK_ID为takIndex的进程，并销毁该进程
     if (takIndex < 0 || takIndex >= TASK_NUM) {
          myPrintk(0x2, "Invalid task index: %d\n", takIndex);
          return; //无效的任务索引
     }
     
     myTCB *tsk = &tcbPool[takIndex]; //获取对应的TCB
     if (tsk->TSK_State == TSK_NONE) {
          myPrintk(0x2, "Task %d is not allocated.\n", takIndex);
          return; //任务未分配，无法销毁
     }

     tsk->TSK_State = TSK_NONE; //将状态设置为未分配
     tsk->nextTCB = firstFreeTsk; //将该TCB加入到空闲链表中
     firstFreeTsk = tsk; //更新下一个空闲的TCB指针
}

unsigned long **prevTSK_StackPtr;
unsigned long *nextTSK_StackPtr;

//切换上下文（无需填写）
void context_switch(myTCB *prevTsk, myTCB *nextTsk) {
     prevTSK_StackPtr = &(prevTsk->stkTop);
     currentTsk = nextTsk;
     nextTSK_StackPtr = nextTsk->stkTop;
     CTX_SW(prevTSK_StackPtr,nextTSK_StackPtr);
}

// FCFS调度，切换至下一个任务
void scheduleFCFS(void) {
     myTCB *nextTsk;
     nextTsk = nextFCFSTsk();
     context_switch(currentTsk,nextTsk);
}

//调度算法（无需填写）
void schedule(void) {
     scheduleFCFS();
}

//进入多任务调度模式(无需填写)
unsigned long BspContextBase[STACK_SIZE];              // BspContextBase 是一块为“主控线程/主程序”（即OS启动时的主流程）分配的专用栈空间。
unsigned long *BspContext;
void startMultitask(void) {
     BspContext = BspContextBase + STACK_SIZE -1;
     prevTSK_StackPtr = &BspContext;                   // 伪装“当前任务”的栈顶指针为主控流程的栈顶
     currentTsk = nextFCFSTsk();                       // 选出下一个要运行的任务（通常是 init 任务或 idle 任务）
     nextTSK_StackPtr = currentTsk->stkTop;
     CTX_SW(prevTSK_StackPtr,nextTSK_StackPtr);        // 切换到下一个任务的上下文
}

//准备进入多任务调度模式(无需填写)
void TaskManagerInit(void) {
     // 初始化进程池（所有的进程状态都是TSK_NONE）
     int i;
     myTCB * thisTCB;
     for(i=0;i<TASK_NUM;i++){                               //对进程池tcbPool中的进程进行初始化处理
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
     createTsk(initTskBody);
    
     //进入多任务状态
     myPrintk(0x2,"START MULTITASKING......\n");
     startMultitask();
     myPrintk(0x2,"STOP MULTITASKING......SHUT DOWN\n");

}
