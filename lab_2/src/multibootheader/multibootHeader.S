.globl start

MAGIC = 0x1BADB002
FLAGS = 0
CHECKSUM = -(MAGIC + FLAGS)   #(magic + checksum + flags should equal 0)

.section ".multiboot_header"
.align 4
  .long MAGIC
  .long FLAGS
  .long CHECKSUM

.text
.code32
start:
  call _start
  hlt  