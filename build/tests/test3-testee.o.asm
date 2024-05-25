	.text
	.file	"my compiler!!!"
	.globl	test3add
	.p2align	4, 0x90
	.type	test3add,@function
test3add:
	leaq	(%rdi,%rsi), %rax
	retq
.Lfunc_end0:
	.size	test3add, .Lfunc_end0-test3add

	.globl	test3
	.p2align	4, 0x90
	.type	test3,@function
test3:
	movq	32(%rdi), %rax
	movq	%rax, -8(%rsp)
	movups	(%rdi), %xmm0
	movups	16(%rdi), %xmm1
	movaps	%xmm1, -24(%rsp)
	movaps	%xmm0, -40(%rsp)
	xorl	%eax, %eax
	testq	%rsi, %rsi
	jle	.LBB1_5
	xorl	%ecx, %ecx
	.p2align	4, 0x90
.LBB1_2:
	movq	-40(%rsp,%rcx,8), %rdx
	xorl	%edi, %edi
	.p2align	4, 0x90
.LBB1_3:
	addq	%rdx, %rax
	addq	-40(%rsp,%rdi,8), %rax
	incq	%rdi
	cmpq	%rsi, %rdi
	jl	.LBB1_3
	incq	%rcx
	cmpq	%rsi, %rcx
	jl	.LBB1_2
.LBB1_5:
	retq
.Lfunc_end1:
	.size	test3, .Lfunc_end1-test3

	.globl	av
	.p2align	4, 0x90
	.type	av,@function
av:
	movq	%rdi, %rax
	movq	$1, (%rdi)
	movq	$1, 8(%rdi)
	movq	$1, 16(%rdi)
	movq	$1, 24(%rdi)
	movq	$1, 32(%rdi)
	retq
.Lfunc_end2:
	.size	av, .Lfunc_end2-av

	.globl	refs
	.p2align	4, 0x90
	.type	refs,@function
refs:
	movl	$100, %eax
	retq
.Lfunc_end3:
	.size	refs, .Lfunc_end3-refs

	.section	".note.GNU-stack","",@progbits
