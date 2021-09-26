.globl g_libvizzy
.globl g_libvizzy_size

g_libvizzy:
    .incbin "./build/libvizzy.so"

g_libvizzy_size:
   .int g_libvizzy_size - g_libvizzy
