format ELF
include "__lib__.inc"
fun      equ __func@vprintf
fun_str  equ 'vprintf'
section '.text'
fun_name db fun_str, 0
section '.data'
extrn lib_name
public fun as fun_str
fun dd fun_name
lib dd lib_name
