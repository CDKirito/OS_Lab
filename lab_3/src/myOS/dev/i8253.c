#include "io.h"

void init8253(void){
	//TODO 你需要填写它
	outb(0x43, 0x34);
    unsigned short divisor = 11932;
    outb(0x40, divisor & 0xFF);
    outb(0x40, divisor >> 8);

	// 解除8253的中断屏蔽
	outb(0x21, inb(0x21) & 0xFE);
}
