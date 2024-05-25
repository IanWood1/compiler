	.text
	.file	"my compiler!!!"
	.globl	add
	.p2align	4, 0x90
	.type	add,@function
add:
	leaq	(%rdi,%rsi), %rax
	retq
.Lfunc_end0:
	.size	add, .Lfunc_end0-add

	.globl	elemAdd
	.p2align	4, 0x90
	.type	elemAdd,@function
elemAdd:
	pushq	%rax
	movq	%r8, %rax
	movq	32(%rdi), %r8
	movq	%r8, (%rsp)
	movups	(%rdi), %xmm0
	movups	16(%rdi), %xmm1
	movaps	%xmm1, -16(%rsp)
	movaps	%xmm0, -32(%rsp)
	movups	(%rsi), %xmm0
	movups	16(%rsi), %xmm1
	movaps	%xmm0, -80(%rsp)
	movaps	%xmm1, -64(%rsp)
	movq	32(%rsi), %rsi
	movq	%rsi, -48(%rsp)
	movups	(%rdx), %xmm0
	movups	16(%rdx), %xmm1
	movaps	%xmm0, -128(%rsp)
	movaps	%xmm1, -112(%rsp)
	movq	32(%rdx), %rdx
	movq	%rdx, -96(%rsp)
	testq	%rcx, %rcx
	jle	.LBB1_3
	xorl	%edx, %edx
	.p2align	4, 0x90
.LBB1_2:
	movq	-32(%rsp,%rdx,8), %rsi
	addq	-80(%rsp,%rdx,8), %rsi
	movq	%rsi, -128(%rsp,%rdx,8)
	incq	%rdx
	cmpq	%rcx, %rdx
	jl	.LBB1_2
.LBB1_3:
	movq	-96(%rsp), %rcx
	movq	%rcx, 32(%rax)
	movaps	-128(%rsp), %xmm0
	movaps	-112(%rsp), %xmm1
	movups	%xmm1, 16(%rax)
	movups	%xmm0, (%rax)
	popq	%rcx
	retq
.Lfunc_end1:
	.size	elemAdd, .Lfunc_end1-elemAdd

	.globl	memcpyArr
	.p2align	4, 0x90
	.type	memcpyArr,@function
memcpyArr:
	pushq	%rbx
	subq	$96, %rsp
	movq	%rcx, %rbx
	movq	32(%rdi), %rax
	movq	%rax, 32(%rsp)
	movups	(%rdi), %xmm0
	movups	16(%rdi), %xmm1
	movaps	%xmm1, 16(%rsp)
	movaps	%xmm0, (%rsp)
	movups	(%rsi), %xmm0
	movups	16(%rsi), %xmm1
	movaps	%xmm0, 48(%rsp)
	movaps	%xmm1, 64(%rsp)
	movq	32(%rsi), %rax
	movq	%rax, 80(%rsp)
	testq	%rdx, %rdx
	jle	.LBB2_2
	shlq	$3, %rdx
	movq	%rsp, %rdi
	leaq	48(%rsp), %rsi
	callq	memcpy@PLT
.LBB2_2:
	movq	32(%rsp), %rax
	movq	%rax, 32(%rbx)
	movaps	(%rsp), %xmm0
	movaps	16(%rsp), %xmm1
	movups	%xmm1, 16(%rbx)
	movups	%xmm0, (%rbx)
	movq	%rbx, %rax
	addq	$96, %rsp
	popq	%rbx
	retq
.Lfunc_end2:
	.size	memcpyArr, .Lfunc_end2-memcpyArr

	.globl	sum
	.p2align	4, 0x90
	.type	sum,@function
sum:
	movq	32(%rdi), %rax
	movq	%rax, -8(%rsp)
	movups	(%rdi), %xmm0
	movups	16(%rdi), %xmm1
	movaps	%xmm1, -24(%rsp)
	movaps	%xmm0, -40(%rsp)
	testq	%rsi, %rsi
	jle	.LBB3_1
	xorl	%ecx, %ecx
	xorl	%eax, %eax
	.p2align	4, 0x90
.LBB3_3:
	addq	-40(%rsp,%rcx,8), %rax
	incq	%rcx
	cmpq	%rsi, %rcx
	jl	.LBB3_3
	retq
.LBB3_1:
	xorl	%eax, %eax
	retq
.Lfunc_end3:
	.size	sum, .Lfunc_end3-sum

	.globl	swap
	.p2align	4, 0x90
	.type	swap,@function
swap:
	retq
.Lfunc_end4:
	.size	swap, .Lfunc_end4-swap

	.globl	doNothingButTestParse
	.p2align	4, 0x90
	.type	doNothingButTestParse,@function
doNothingButTestParse:
	movl	$18, %eax
	retq
.Lfunc_end5:
	.size	doNothingButTestParse, .Lfunc_end5-doNothingButTestParse

	.globl	test1
	.p2align	4, 0x90
	.type	test1,@function
test1:
	pushq	%r15
	pushq	%r14
	pushq	%rbx
	subq	$288, %rsp
	movq	%rdx, %rbx
	movq	%rsi, %r14
	xorps	%xmm2, %xmm2
	movaps	%xmm2, (%rsp)
	movaps	%xmm2, 16(%rsp)
	movq	$0, 32(%rsp)
	movups	(%rdi), %xmm0
	movups	16(%rdi), %xmm1
	movaps	%xmm0, 48(%rsp)
	movaps	%xmm1, 64(%rsp)
	movq	32(%rdi), %rax
	movq	%rax, 80(%rsp)
	testq	%rdx, %rdx
	jle	.LBB6_2
	leaq	(,%rbx,8), %r15
	movq	%rsp, %rdi
	leaq	48(%rsp), %rsi
	movq	%r15, %rdx
	callq	memcpy@PLT
	movaps	(%rsp), %xmm0
	movaps	16(%rsp), %xmm1
	movaps	%xmm0, 96(%rsp)
	movaps	%xmm1, 112(%rsp)
	movq	32(%rsp), %rax
	movq	%rax, 128(%rsp)
	xorps	%xmm0, %xmm0
	movaps	%xmm0, (%rsp)
	movaps	%xmm0, 16(%rsp)
	movq	$0, 32(%rsp)
	movups	(%r14), %xmm0
	movups	16(%r14), %xmm1
	movaps	%xmm0, 48(%rsp)
	movaps	%xmm1, 64(%rsp)
	movq	32(%r14), %rax
	movq	%rax, 80(%rsp)
	movq	%rsp, %rdi
	leaq	48(%rsp), %rsi
	movq	%r15, %rdx
	callq	memcpy@PLT
	jmp	.LBB6_3
.LBB6_2:
	movq	32(%rsp), %rax
	movq	%rax, 128(%rsp)
	movaps	(%rsp), %xmm0
	movaps	16(%rsp), %xmm1
	movaps	%xmm1, 112(%rsp)
	movaps	%xmm0, 96(%rsp)
	movaps	%xmm2, (%rsp)
	movaps	%xmm2, 16(%rsp)
	movq	$0, 32(%rsp)
	movups	(%r14), %xmm0
	movups	16(%r14), %xmm1
	movaps	%xmm0, 48(%rsp)
	movaps	%xmm1, 64(%rsp)
	movq	32(%r14), %rax
	movq	%rax, 80(%rsp)
.LBB6_3:
	movq	32(%rsp), %rax
	movq	%rax, 272(%rsp)
	movaps	(%rsp), %xmm0
	movaps	16(%rsp), %xmm1
	movaps	%xmm1, 256(%rsp)
	movaps	%xmm0, 240(%rsp)
	movaps	96(%rsp), %xmm2
	movaps	112(%rsp), %xmm3
	movaps	%xmm2, (%rsp)
	movaps	%xmm3, 16(%rsp)
	movq	128(%rsp), %rcx
	movq	%rcx, 32(%rsp)
	movaps	%xmm0, 48(%rsp)
	movaps	%xmm1, 64(%rsp)
	movq	%rax, 80(%rsp)
	xorps	%xmm0, %xmm0
	movaps	%xmm0, 144(%rsp)
	movaps	%xmm0, 160(%rsp)
	movq	$0, 176(%rsp)
	testq	%rbx, %rbx
	jle	.LBB6_9
	xorl	%eax, %eax
	.p2align	4, 0x90
.LBB6_5:
	movq	(%rsp,%rax,8), %rcx
	addq	48(%rsp,%rax,8), %rcx
	movq	%rcx, 144(%rsp,%rax,8)
	incq	%rax
	cmpq	%rbx, %rax
	jl	.LBB6_5
	movq	176(%rsp), %rax
	movq	%rax, 224(%rsp)
	movaps	144(%rsp), %xmm0
	movaps	160(%rsp), %xmm1
	movaps	%xmm1, 208(%rsp)
	movaps	%xmm0, 192(%rsp)
	movq	%rax, 32(%rsp)
	movaps	%xmm1, 16(%rsp)
	movaps	%xmm0, (%rsp)
	xorl	%ecx, %ecx
	xorl	%eax, %eax
	.p2align	4, 0x90
.LBB6_7:
	addq	(%rsp,%rcx,8), %rax
	incq	%rcx
	cmpq	%rbx, %rcx
	jl	.LBB6_7
	jmp	.LBB6_8
.LBB6_9:
	movaps	%xmm0, 208(%rsp)
	movaps	%xmm0, 192(%rsp)
	movq	$0, 224(%rsp)
	xorl	%eax, %eax
.LBB6_8:
	addq	$288, %rsp
	popq	%rbx
	popq	%r14
	popq	%r15
	retq
.Lfunc_end6:
	.size	test1, .Lfunc_end6-test1

	.globl	fasdfasdfsa
	.p2align	4, 0x90
	.type	fasdfasdfsa,@function
fasdfasdfsa:
	xorl	%eax, %eax
	retq
.Lfunc_end7:
	.size	fasdfasdfsa, .Lfunc_end7-fasdfasdfsa

	.section	".note.GNU-stack","",@progbits
