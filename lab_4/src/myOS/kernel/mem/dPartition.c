#include "../../include/myPrintk.h"


//dPartition 是整个动态分区内存的数据结构
typedef struct dPartition{
	unsigned long size;
	unsigned long firstFreeStart; 
} dPartition;	//共占8个字节

#define dPartition_size ((unsigned long)0x8)

void showdPartition(struct dPartition *dp){
	myPrintk(0x5,"dPartition(start=0x%x, size=0x%x, firstFreeStart=0x%x)\n", dp, dp->size,dp->firstFreeStart);
}

// EMB 是每一个block的数据结构，userdata可以暂时不用管。
typedef struct EMB{
	unsigned long size;
	union {
		unsigned long nextStart;    // if free: pointer to next block
        unsigned long userData;		// if allocated, belongs to user
	};	                           
} EMB;	//共占8个字节

#define EMB_size ((unsigned long)0x8)

// 隔离带
#define GUARD_SIZE 4
#define GUARD_VALUE 0xDEADBEEF

void showEMB(struct EMB * emb){
	myPrintk(0x3,"EMB(start=0x%x, size=0x%x, nextStart=0x%x)\n", emb, emb->size, emb->nextStart);
}


unsigned long dPartitionInit(unsigned long start, unsigned long totalSize){
	// TODO
	/*功能：初始化内存。
	1. 在地址start处，首先是要有dPartition结构体表示整个数据结构(也即句柄)。
	2. 然后，一整块的EMB被分配（以后使用内存会逐渐拆分），在内存中紧紧跟在dP后面，然后dP的firstFreeStart指向EMB。
	3. 返回start首地址(也即句柄)。
	注意有两个地方的大小问题：
		第一个是由于内存肯定要有一个EMB和一个dPartition，totalSize肯定要比这两个加起来大。
		第二个注意EMB的size属性不是totalsize，因为dPartition和EMB自身都需要要占空间。
	*/
	
	totalSize &= ~0x7; // 8字节对齐

    // 检查空间是否足够
    if (totalSize < dPartition_size + EMB_size) {
        myPrintk(0x7, "dPartitionInit: totalSize too small!\n");
        return 0;
    }

    // 1. 初始化 dPartition 结构体
    dPartition *dp = (dPartition *)start;
    dp->size = totalSize;
    dp->firstFreeStart = start + dPartition_size;

    // 2. 初始化第一个 EMB（空闲块）
    EMB *firstEMB = (EMB *)(start + dPartition_size);
    firstEMB->size = totalSize - dPartition_size; // EMB 占用的总空间
    firstEMB->nextStart = 0; // 目前只有一个空闲块

    // 3. 返回 dPartition 的首地址（句柄）
    return start;
}


void dPartitionWalkByAddr(unsigned long dp){
	// TODO
	/*功能：本函数遍历输出EMB 方便调试
	1. 先打印dP的信息，可调用上面的showdPartition。
	2. 然后按地址的大小遍历EMB，对于每一个EMB，可以调用上面的showEMB输出其信息
	*/
	showdPartition((struct dPartition *)dp);
	unsigned long current = ((struct dPartition *)dp)->firstFreeStart;
	while (current != 0) {
		struct EMB *emb = (struct EMB *)current;
		showEMB(emb);
		current = emb->nextStart; // 获取下一个空闲块的起始地址
	}
}
       

//=================firstfit, order: address, low-->high=====================
/**
 * return value: addr (without overhead, can directly used by user)
**/

unsigned long dPartitionAllocFirstFit(unsigned long dp, unsigned long size){
	// TODO
	/*功能：分配一个空间
	1. 使用firstfit的算法分配空间，
	2. 成功分配返回首地址，不成功返回0
	3. 从空闲内存块组成的链表中拿出一块供我们来分配空间(如果提供给分配空间的内存块空间大于size，我们还将把剩余部分放回链表中)，并维护相应的空闲链表以及句柄
	注意的地方：
		1.EMB类型的数据的存在本身就占用了一定的空间。
	*/
	
	// 8字节对齐
    size = (size + 0x7) & ~0x7;
	// 需要总空间 = 用户数据大小 + 前后隔离带 + EMB 结构体大小
    unsigned long needSize = size + 2 * GUARD_SIZE + EMB_size;

	dPartition *dP = (dPartition *)dp;
    unsigned long *prevPtr = &(dP->firstFreeStart);
    unsigned long current = dP->firstFreeStart;

    while (current != 0) {
        EMB *emb = (EMB *)current;
        if (emb->size >= needSize) {
            // 如果剩余空间足够分割出一个新的EMB，则拆分（8为最小分配单位）
            if (emb->size >= needSize + EMB_size + 8) {
                unsigned long nextFree = current + needSize;
                EMB *newEMB = (EMB *)nextFree;
                newEMB->size = emb->size - needSize;
                newEMB->nextStart = emb->nextStart;

                emb->size = needSize;
                emb->nextStart = 0;
                *prevPtr = nextFree;
            } else {
                // 否则整块分配
                *prevPtr = emb->nextStart;
            }

            // 写入隔离带
            unsigned char *userPtr = (unsigned char *)emb + EMB_size + GUARD_SIZE;
            *(unsigned int *)((unsigned char *)emb + EMB_size) = GUARD_VALUE; // 前隔离带
            *(unsigned int *)(userPtr + size) = GUARD_VALUE;                  // 后隔离带

            // 返回用户可用空间首地址
            return (unsigned long)userPtr;
        }
        prevPtr = &(emb->nextStart);
        current = emb->nextStart;
    }
    // 没有合适空间
    return 0;
}


unsigned long dPartitionFreeFirstFit(unsigned long dp, unsigned long start){
	// TODO
	/*功能：释放一个空间
	1. 按照对应的fit的算法释放空间
	2. 注意检查要释放的start~end这个范围是否在dp有效分配范围内
		返回1 没问题
		返回0 error
	3. 需要考虑两个空闲且相邻的内存块的合并
	*/
	
    dPartition *dP = (dPartition *)dp;
    unsigned long poolStart = dp + dPartition_size;	 // 可用内存池的起始地址
    unsigned long poolEnd = dp + dP->size;			 // 可用内存池的结束地址

    // 找到EMB头部
    EMB *emb = (EMB *)(start - GUARD_SIZE - EMB_size);

    // 检查释放范围是否合法
    unsigned long embStart = (unsigned long)emb;
    unsigned long embEnd = embStart + emb->size;
    if (embStart < poolStart || embEnd > poolEnd) {
        myPrintk(0x7, "dPartitionFreeFirstFit: free range invalid!\n");
        return 0;
    }

    // 检查隔离带
    unsigned int *frontGuard = (unsigned int *)((unsigned char *)emb + EMB_size);
    unsigned int *rearGuard = (unsigned int *)(start + (emb->size - EMB_size - 2 * GUARD_SIZE));
    if (*frontGuard != GUARD_VALUE) {
        myPrintk(0x7, "dPartitionFreeFirstFit: front guard corrupted!\n");
        return 0;
    }
    if (*rearGuard != GUARD_VALUE) {
        myPrintk(0x7, "dPartitionFreeFirstFit: rear guard corrupted!\n");
        return 0;
    }

    // 插入空闲链表（按地址有序插入）
    unsigned long *prevPtr = &(dP->firstFreeStart);
    unsigned long current = dP->firstFreeStart;
    while (current != 0 && current < embStart) {
        prevPtr = &(((EMB *)current)->nextStart);
        current = ((EMB *)current)->nextStart;
    }
    emb->nextStart = current;
    *prevPtr = embStart;

    // 合并前后相邻空闲块
    // 合并后面的
    if (emb->nextStart != 0) {
        EMB *nextEMB = (EMB *)(emb->nextStart);
        if (embStart + emb->size == emb->nextStart) {
            emb->size += nextEMB->size;
            emb->nextStart = nextEMB->nextStart;
        }
    }
    // 合并前面的
	if (prevPtr != &(dP->firstFreeStart)) {
		// 通过遍历链表找到前一个EMB
		EMB *prevEMB = (EMB *)(dP->firstFreeStart);
		while (prevEMB && prevEMB->nextStart != embStart) {
			prevEMB = (EMB *)(prevEMB->nextStart);
		}
		if (prevEMB && (unsigned long)prevEMB + prevEMB->size == embStart) {
			prevEMB->size += emb->size;
			prevEMB->nextStart = emb->nextStart;
		}
	}

    return 1;
}


// 进行封装，此处默认firstfit分配算法，当然也可以使用其他fit，不限制。
unsigned long dPartitionAlloc(unsigned long dp, unsigned long size){
	return dPartitionAllocFirstFit(dp,size);
}

unsigned long dPartitionFree(unsigned long	 dp, unsigned long start){
	return dPartitionFreeFirstFit(dp,start);
}
