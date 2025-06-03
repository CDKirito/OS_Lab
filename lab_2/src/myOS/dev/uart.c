/* 
* 与 UART 相关的输出
* 调用inb和outb函数，实现下面的uart的三个函数
*/
extern unsigned char inb(unsigned short int port_from);
extern void outb (unsigned short int port_to, unsigned char value);

#define uart_base 0x3F8     // 串口基地址

void uart_put_char(unsigned char c){            // 向串口发送一个字符
    while ((inb(uart_base + 5) & 0x20) == 0);   // 等待发送缓冲区空
    outb(uart_base, c);                         // 发送字符
}

unsigned char uart_get_char(void){              // 从串口接收一个字符
    while ((inb(uart_base + 5) & 0x01) == 0);   // 等待接收缓冲区有数据
    return inb(uart_base);                      // 读取字符 并返回
}

void uart_put_chars(char *str){                 // 向串口发送字符串
    while(*str) {
        uart_put_char(*str++);
    }
}