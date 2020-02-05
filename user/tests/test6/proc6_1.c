#if defined microblaze
__asm__(
".text\n"
".globl main\n"
"main:\n"
"addik r3,r0,3\n"
"rtsd r15,8\n"
"nop\n"
".previous\n"
);
#else
__asm__(
".text\n"
".globl main\n"
"main:\n"
"movl $3,%eax\n"
"ret\n"
".previous\n"
);
#endif
