STACK_SIZE =	0x1000	# Size of heap and stack. 4KB

/* ===== code32 =========================== */

	.globl 	_start		# GNU default entry point
	.globl 	osStart
.text
.code32
_start:
	jmp establish_stack

dead:	jmp dead			# Never here

# Set up the stack
establish_stack:
	; movl	_end, %eax		# eax = end of bss/start of heap #填入正确的内容
	; addl	$STACK_SIZE, %eax	# make room for stack
	; andl	$0xffffffc0, %eax	# align it on 16 byte boundary
	movl	$0x80000, %eax	
	
	movl	%eax, %esp		# set stack pointer
	movl	%eax, %ebp		# set base pointer

# Zero out the BSS segment
zero_bss:
	cld				              # make direction flag count up
	movl	$_end, %ecx		    # find end of .bss
	movl	$_bss_start, %edi	# edi = beginning of .bss
	subl	%edi, %ecx		    # ecx = size of .bss in bytes
	shrl	%ecx			        # size of .bss in longs
	shrl	%ecx
	
	xorl	%eax, %eax		    # value to clear out memory
	repne			            	# while ecx != 0
	stosl				            # clear a long in the bss

	
setup_idt:
	movl $ignore_int1,%edx
	movl $0x00080000,%eax
	movw %dx,%ax /* selector = 0x0010 = cs */
	movw $0x8E00,%dx /* interrupt gate - dpl=0, present */
	movl $IDT,%edi
	mov $256,%ecx
	
rp_sidt:
	movl %eax,(%edi)
	movl %edx,4(%edi)
	addl $8,%edi
	dec %ecx
	jne rp_sidt
	
	lidt idtptr
	
	call setup_time_int_32
	
# Transfer control to main
to_main:	
	call	osStart

shut_down:	
	jmp	shut_down	# Never here	

	.p2align 4
time_interrupt:
# TODO 你需要填写它
	cld
	pushf
	pusha
	call tick
	popa
	popf
	iret
	
	.p2align 4
ignore_int1:			# 中断入口处理程序的符号名
# TODO 你需要填写它
	cld					# 将标志寄存器 eflags 的方向标志位 DF 清 0
	pusha				# 保存寄存器上下文
	call ignoreIntBody
	popa				# 恢复寄存器上下文
	iret				# 中断返回
	

# ret /* if do not set timer*/
setup_time_int_32:
	movl $time_interrupt,%edx
	movl $0x00080000,%eax /* selector：0x0010 = cs */
	movw %dx,%ax
	movw $0x8E00,%dx /* interrupt gate - dpl=0, present */
	movl $IDT,%edi
	addl $(32*8), %edi
	movl %eax,(%edi)
	movl %edx,4(%edi)
	ret


/* ===== data =========================== */
.data
# IDT
	.p2align 4		# 确保 IDT 的起始地址是 4 字节对齐的
	.globl IDT		# 声明 IDT 为全局符号，使其在其他文件中也可以访问
IDT:
	.rept 256		# 重复执行接下来的指令 256 次
	.word 0,0,0,0	# 定义一个 4 字节的字，并将每个字初始化为 0
	.endr			# 结束重复指令

# lidt idtptr 会把 idtptr 里的内容加载到CPU的IDTR寄存器，告诉CPU中断描述符表的位置和大小。
idtptr:
	.word (256*8 - 1)	# IDT的界限（字节数-1），共256项，每项8字节
	.long IDT			# IDT的起始地址
