This is the baseline files for the ARM ISA simulator.

Memory Map is as follow (in shell.c)

#define MEM_DATA_START  0x10000000<br>
#define MEM_DATA_SIZE   0x00100000<br>
#define MEM_TEXT_START  0x00400000<br>
#define MEM_TEXT_SIZE   0x00100000<br>
#define MEM_STACK_START 0x7ff00000<br>
#define MEM_STACK_SIZE  0x00100000<br>
#define MEM_KDATA_START 0x90000000<br>
#define MEM_KDATA_SIZE  0x00100000<br>
#define MEM_KTEXT_START 0x80000000<br>
#define MEM_KTEXT_SIZE  0x00100000<br>

Basically memory starts at 0x1000_0000<br>
Program loads into 0x0040_0000<br>




