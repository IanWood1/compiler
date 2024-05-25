	.text
	.file	"my compiler!!!"
	.globl	minitest_forward_const
	.p2align	4, 0x90
	.type	minitest_forward_const,@function
minitest_forward_const:
	movq	%rdi, %rax
	retq
.Lfunc_end0:
	.size	minitest_forward_const, .Lfunc_end0-minitest_forward_const

	.globl	minitest1
	.p2align	4, 0x90
	.type	minitest1,@function
minitest1:
	movl	$13, %eax
	retq
.Lfunc_end1:
	.size	minitest1, .Lfunc_end1-minitest1

	.globl	minitest_custom_insert
	.p2align	4, 0x90
	.type	minitest_custom_insert,@function
minitest_custom_insert:
	movq	(%rdi), %rax
	movq	%rdx, (%rax,%rsi,8)
	retq
.Lfunc_end2:
	.size	minitest_custom_insert, .Lfunc_end2-minitest_custom_insert

	.globl	minitest2
	.p2align	4, 0x90
	.type	minitest2,@function
minitest2:
	movl	$115, %eax
	retq
.Lfunc_end3:
	.size	minitest2, .Lfunc_end3-minitest2

	.globl	minitest3
	.p2align	4, 0x90
	.type	minitest3,@function
minitest3:
	movl	$15, %eax
	retq
.Lfunc_end4:
	.size	minitest3, .Lfunc_end4-minitest3

	.globl	minitest_retArrayByVal
	.p2align	4, 0x90
	.type	minitest_retArrayByVal,@function
minitest_retArrayByVal:
	movq	%rsi, %rax
	movq	%rdi, (%rsi)
	movq	%rdi, 8(%rsi)
	movq	%rdi, 16(%rsi)
	movq	%rdi, 24(%rsi)
	movq	%rdi, 32(%rsi)
	movq	%rdi, 40(%rsi)
	movq	%rdi, 48(%rsi)
	movq	%rdi, 56(%rsi)
	movq	%rdi, 64(%rsi)
	movq	%rdi, 72(%rsi)
	retq
.Lfunc_end5:
	.size	minitest_retArrayByVal, .Lfunc_end5-minitest_retArrayByVal

	.globl	minitest4
	.p2align	4, 0x90
	.type	minitest4,@function
minitest4:
	movl	$10, %eax
	retq
.Lfunc_end6:
	.size	minitest4, .Lfunc_end6-minitest4

	.globl	minitest_local_mutate
	.p2align	4, 0x90
	.type	minitest_local_mutate,@function
minitest_local_mutate:
	retq
.Lfunc_end7:
	.size	minitest_local_mutate, .Lfunc_end7-minitest_local_mutate

	.globl	minitest_ref_mutate
	.p2align	4, 0x90
	.type	minitest_ref_mutate,@function
minitest_ref_mutate:
	movq	(%rdi), %rax
	movq	$100, (%rax)
	retq
.Lfunc_end8:
	.size	minitest_ref_mutate, .Lfunc_end8-minitest_ref_mutate

	.globl	minitest5
	.p2align	4, 0x90
	.type	minitest5,@function
minitest5:
	xorl	%eax, %eax
	retq
.Lfunc_end9:
	.size	minitest5, .Lfunc_end9-minitest5

	.section	".note.GNU-stack","",@progbits
