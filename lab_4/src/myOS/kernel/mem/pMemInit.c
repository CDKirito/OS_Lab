#include "../../include/myPrintk.h"
#include "../../include/mem.h"
unsigned long pMemStart;  // 可用的内存的起始地址
unsigned long pMemSize;  // 可用的大小

void memTest(unsigned long start, unsigned long grainSize){
	// TODO
	/*功能：检测算法
		这一个函数对应实验讲解ppt中的第一大功能-内存检测。
		本函数的功能是检测从start开始有多大的内存可用，具体算法参照ppt检测算法的文字描述
	注意点三个：
	1、覆盖写入和读出就是往指针指向的位置写和读，不要想复杂。
	  (至于为什么这种检测内存的方法可行大家要自己想一下)
	2、开始的地址要大于1M，需要做一个if判断。
	3、grainsize不能太小，也要做一个if判断
	*/

	if (start < 0x100000) start = 0x100000;  // 确保起始地址大于1M
	if (grainSize < 0x100) grainSize = 0x100;  // 确保粒度不小于256字节

	unsigned long size = 0;
    int fail = 0;
    while (!fail) {
		unsigned short *grainHead = (unsigned short *)(start + size);
		unsigned short *grainTail = (unsigned short *)(start + size + grainSize - 2);

		// 检查grain头部2字节
		unsigned short oldHead = *grainHead;

		*grainHead = 0xAA55;
		if (*grainHead != 0xAA55) { fail = 1; break; }

		*grainHead = 0x55AA;
		if (*grainHead != 0x55AA) { fail = 1; break; }

		*grainHead = oldHead; // 恢复原值

		// 检查grain尾部2字节
		unsigned short oldTail = *grainTail;

		*grainTail = 0xAA55;
		if (*grainTail != 0xAA55) { fail = 1; break; }

		*grainTail = 0x55AA;
		if (*grainTail != 0x55AA) { fail = 1; break; }

		*grainTail = oldTail; // 恢复原值

		size += grainSize;
	}

	pMemStart = start;
	pMemSize = size;

    myPrintk(0x7,"MemStart: %x  \n", start);
    myPrintk(0x7,"MemSize:  %x  \n", size);
}

extern unsigned long _end;
unsigned long pMemHandler;
void pMemInit(void){
	unsigned long _end_addr = (unsigned long) &_end;
	memTest(0x100000,0x1000);
	myPrintk(0x7,"_end:  %x  \n", _end_addr);
	if (pMemStart <= _end_addr) {
		pMemSize -= _end_addr - pMemStart;
		pMemStart = _end_addr;
	}
	
	// TODO:此处选择不同的内存管理算法
	pMemHandler = dPartitionInit(pMemStart,pMemSize);	
}
