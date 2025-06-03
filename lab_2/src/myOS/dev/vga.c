/*
* 本文件实现vga的相关功能，清屏和屏幕输出
* clear_screen和append2screen必须按照如下借口实现
* 可以增加其他函数供clear_screen和append2screen调用
*/
extern void outb (unsigned short int port_to, unsigned char value);
extern unsigned char inb(unsigned short int port_from);
//VGA字符界面规格：25行80列
//VGA显存初始地址为0xB8000

short cur_line=0;
short cur_column=0;//当前光标位置
char *vga_mem = (char *)0xB8000;

void update_cursor(void){//通过当前行值cur_cline与列值cur_column回写光标
    unsigned short position = cur_line * 80 + cur_column;

    outb(0x3D4, 0x0F);
    outb(0x3D5, (unsigned char)(position & 0xFF));
    outb(0x3D4, 0x0E);
    outb(0x3D5, (unsigned char)((position >> 8) & 0xFF));
}

short get_cursor_position(void){//获得当前光标，计算出cur_line和cur_column的值
    unsigned char low, high;
    short position;

    outb(0x3D4, 0x0F);
    low = inb(0x3D5);
    outb(0x3D4, 0x0E);
    high = inb(0x3D5);

    position = (high << 8) | low;
    cur_line = position / 80;
    cur_column = position % 80;

    return position;
}

void clear_screen(void) {
    for(int i = 0; i < 25 * 80; i++) {
        vga_mem[i * 2] = ' ';
        vga_mem[i * 2 + 1] = 0x07;
    }
    cur_line = 0;
    cur_column = 0;
    update_cursor();
}

void append2screen(char *str,int color) {
    while(*str != '\0') {
        if((*str == '\n') || (cur_column == 80)) {
            // 换行操作
            cur_column = 0;
            cur_line++;
            if (cur_column == 80) {
                str--;
            }
            if (cur_line >= 25) {
                // 滚屏操作
                for(int i = 0; i < 24 * 80; i++) {
                    vga_mem[i * 2] = vga_mem[(i + 80) * 2];
                    vga_mem[i * 2 + 1] = vga_mem[(i + 80) * 2 + 1];
                }
                for(int i = 24 * 80; i < 25 * 80; i++) {
                    vga_mem[i * 2] = ' ';
                    vga_mem[i * 2 + 1] = 0x07;
                }
                cur_line = 24;
            }
            update_cursor();
        } else {
            vga_mem[(cur_line * 80 + cur_column) * 2] = *str;
            vga_mem[(cur_line * 80 + cur_column) * 2 + 1] = color;
            cur_column++;
            update_cursor();
        }
        str++;
    }
}