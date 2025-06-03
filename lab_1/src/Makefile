# 定义变量
ASM_FLAGS = -m32 --pipe -Wall -fasm -g -O1 -fno-stack-protector
# -m32：指定目标架构为32位。
# --pipe：使用管道而不是临时文件进行编译过程中的中间步骤，这可以提高编译速度。
# -Wall：启用所有警告信息，帮助发现潜在的代码问题。
# -fasm：指定使用汇编语言作为中间表示（这通常是默认行为，但显式指定可以避免混淆）。
# -g：生成调试信息，便于后续使用调试工具（如 GDB）。
# -O1：优化级别为1，表示在代码大小和执行速度之间进行平衡的优化。
# -fno-stack-protector：禁用栈保护功能，这在某些低级编程场景中可能有用，但通常不推荐在生产代码中使用，因为它会降低安全性。

# 目标规则：生成 multibootHeader.bin 文件
multibootHeader.bin: multibootHeader.S
# .bin 是最终生成的二进制文件，.S 是源代码文件，这是一个汇编语言文件。

	gcc -c ${ASM_FLAGS} multibootHeader.S -o multibootHeader.o
# 用 gcc 编译器将 multibootHeader.S 汇编代码编译成目标文件 multibootHeader.o。
# -c 表示仅编译生成目标文件，不进行链接。${ASM_FLAGS} 是前面定义的变量

	ld -n -T multibootHeader.ld multibootHeader.o -o multibootHeader.bin
# 使用 ld 链接器将目标文件 multibootHeader.o 链接成最终的二进制文件 multibootHeader.bin。
# -n：告诉链接器不要在可执行文件中创建 .note.gnu.build-id 段。
# -T multibootHeader.ld：指定链接脚本文件 multibootHeader.ld，链接脚本用于定义输出文件的内存布局等信息。
# -o multibootHeader.bin：指定输出文件为 multibootHeader.bin。

# 清理规则
clean:
	rm -rf ./multibootHeader.bin ./multibootHeader.o
# rm -rf：强制删除指定的文件和目录，-r 表示递归删除，-f 表示强制删除，不提示确认。
# ./multibootHeader.bin 和 ./multibootHeader.o 是要删除的文件，./ 表示当前目录。