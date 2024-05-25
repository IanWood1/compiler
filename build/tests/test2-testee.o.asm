	.text
	.file	"my compiler!!!"
	.globl	test2
	.p2align	4, 0x90
	.type	test2,@function
test2:
	leaq	(%rdi,%rsi), %rax
	retq
.Lfunc_end0:
	.size	test2, .Lfunc_end0-test2

	.section	".note.GNU-stack","",@progbits
