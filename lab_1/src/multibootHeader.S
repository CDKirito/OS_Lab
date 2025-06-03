/* 声明 start 标签为全局符号，链接器可以识别它作为程序的入口点 */
.globl start

/* 多引导头的定义 */
/* 魔数，固定值 0x1BADB002，用于标识这是一个多引导兼容的内核 */
MAGIC_ITEM_NAME = 0x1BADB002
/* 标志字段，这里设置为 0，表示没有特殊要求 */
FLAGS_ITEM_NAME = 0
/* 校验和，根据多引导规范，魔数 + 标志 + 校验和必须为 0 */
CHECKSUM_ITEM_NAME = -(MAGIC_ITEM_NAME + FLAGS_ITEM_NAME)

/* 切换到 .multiboot_header 段 */
.section .multiboot_header
    /* 将魔数、标志和校验和写入多引导头 */
    .long MAGIC_ITEM_NAME
    .long FLAGS_ITEM_NAME
    .long CHECKSUM_ITEM_NAME

/* 切换到代码段 */
.text
/* 指定后续代码为 32 位代码 */
.code32
/* 程序入口点 */
start:
    /* VGA 输出 Hello world!PB22030892_LaiKaiyu */
    /* 将字符串写入 VGA 显存（地址 0xb8000），从而在屏幕上显示 */
    movl $0x2f652f48, 0xb8000    /* eH */
    movl $0x2f6C2f6C, 0xb8004    /* ll */
    movl $0x2f202f6F, 0xb8008    /* \space o */
    movl $0x2f6F2f77, 0xb800C    /* ow */
    movl $0x2f6C2f72, 0xb8010    /* lr */
    movl $0x2f212f64, 0xb8014    /* !d */
    movl $0x2f422f50, 0xb8018    /* BP */
    movl $0x2f322f32, 0xb801C    /* 22 */
    movl $0x2f332f30, 0xb8020    /* 30 */
    movl $0x2f382f30, 0xb8024    /* 80 */
    movl $0x2f322f39, 0xb8028    /* 29 */
    movl $0x2f4C2f5F, 0xb802C    /* L_ */
    movl $0x2f752f69, 0xb8030    /* ui */
    movl $0x2f612f4B, 0xb8034    /* aK */
    movl $0x2f792f69, 0xb8038    /* yi */
    movw $0x2f75    , 0xb803C    /* u */

    /* 串口输出 Hello world!PB22030892_LaiKaiyu */
    /* 设置串口地址为 COM1（0x3F8） */
    movw $0x3F8, %dx    
    /* 将字符 'H' 放入 AL 寄存器，并输出到串口 */
    movb $'H', %al
    outb %al, %dx
    /* 重复上述操作，输出整个字符串 */
    movb $'e', %al
    outb %al, %dx
    movb $'l', %al
    outb %al, %dx
    movb $'l', %al
    outb %al, %dx
    movb $'o', %al
    outb %al, %dx
    movb $' ', %al
    outb %al, %dx
    movb $'w', %al
    outb %al, %dx
    movb $'o', %al
    outb %al, %dx
    movb $'r', %al
    outb %al, %dx
    movb $'l', %al
    outb %al, %dx
    movb $'d', %al
    outb %al, %dx
    movb $'!', %al
    outb %al, %dx
    movb $'P', %al
    outb %al, %dx
    movb $'B', %al
    outb %al, %dx
    movb $'2', %al
    outb %al, %dx
    movb $'2', %al
    outb %al, %dx
    movb $'0', %al
    outb %al, %dx
    movb $'3', %al
    outb %al, %dx
    movb $'0', %al
    outb %al, %dx
    movb $'8', %al
    outb %al, %dx
    movb $'9', %al
    outb %al, %dx
    movb $'2', %al
    outb %al, %dx
    movb $'_', %al
    outb %al, %dx
    movb $'L', %al
    outb %al, %dx
    movb $'a', %al
    outb %al, %dx
    movb $'i', %al
    outb %al, %dx
    movb $'K', %al
    outb %al, %dx
    movb $'a', %al
    outb %al, %dx
    movb $'i', %al
    outb %al, %dx
    movb $'y', %al
    outb %al, %dx
    movb $'u', %al
    outb %al, %dx

    /* 停机指令，使 CPU 进入空闲状态 */
    hlt