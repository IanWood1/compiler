	.text
	.def	@feat.00;
	.scl	3;
	.type	0;
	.endef
	.globl	@feat.00
.set @feat.00, 0
	.file	"my compiler!!!"
	.def	add;
	.scl	2;
	.type	32;
	.endef
	.globl	add
	.p2align	4, 0x90
add:
	leaq	(%rcx,%rdx), %rax
	retq

	.def	main;
	.scl	2;
	.type	32;
	.endef
	.globl	main
	.p2align	4, 0x90
main:
	movl	$12, %eax
	retq

	.def	elemAdd;
	.scl	2;
	.type	32;
	.endef
	.globl	elemAdd
	.p2align	4, 0x90
elemAdd:
	movq	%r8, %rax
	testq	%r9, %r9
	jle	.LBB2_3
	xorl	%r8d, %r8d
	.p2align	4, 0x90
.LBB2_2:
	movq	(%rcx,%r8,8), %r10
	addq	(%rdx,%r8,8), %r10
	movq	%r10, (%rax,%r8,8)
	incq	%r8
	cmpq	%r9, %r8
	jl	.LBB2_2
.LBB2_3:
	retq

	.def	memcpyArr;
	.scl	2;
	.type	32;
	.endef
	.globl	memcpyArr
	.p2align	4, 0x90
memcpyArr:
	movq	%rcx, %rax
	testq	%r8, %r8
	jle	.LBB3_3
	xorl	%ecx, %ecx
	.p2align	4, 0x90
.LBB3_2:
	movq	(%rdx,%rcx,8), %r9
	movq	%r9, (%rax,%rcx,8)
	incq	%rcx
	cmpq	%r8, %rcx
	jl	.LBB3_2
.LBB3_3:
	retq

	.def	sum;
	.scl	2;
	.type	32;
	.endef
	.globl	sum
	.p2align	4, 0x90
sum:
	testq	%rdx, %rdx
	jle	.LBB4_1
	xorl	%r8d, %r8d
	xorl	%eax, %eax
	.p2align	4, 0x90
.LBB4_3:
	addq	(%rcx,%r8,8), %rax
	incq	%r8
	cmpq	%rdx, %r8
	jl	.LBB4_3
	retq
.LBB4_1:
	xorl	%eax, %eax
	retq

	.def	swap;
	.scl	2;
	.type	32;
	.endef
	.globl	swap
	.p2align	4, 0x90
swap:
	pushq	%rbp
	pushq	%r15
	pushq	%r14
	pushq	%rsi
	pushq	%rdi
	pushq	%rbx
	pushq	%rax
	movq	%rsp, %rbp
	movq	%r8, %rdi
	movq	%rdx, %rsi
	movq	%rcx, %rbx
	leaq	15(,%r8,8), %rax
	andq	$-16, %rax
	callq	__chkstk
	subq	%rax, %rsp
	movq	%rsp, %r14
	testq	%r8, %r8
	jle	.LBB5_4
	leaq	(,%rdi,8), %r15
	subq	$32, %rsp
	movq	%r14, %rcx
	movq	%rbx, %rdx
	movq	%r15, %r8
	callq	memcpy
	addq	$32, %rsp
	xorl	%eax, %eax
	.p2align	4, 0x90
.LBB5_2:
	movq	(%rsi,%rax,8), %rcx
	movq	%rcx, (%rbx,%rax,8)
	incq	%rax
	cmpq	%rdi, %rax
	jl	.LBB5_2
	subq	$32, %rsp
	movq	%rsi, %rcx
	movq	%r14, %rdx
	movq	%r15, %r8
	callq	memcpy
	addq	$32, %rsp
.LBB5_4:
	leaq	8(%rbp), %rsp
	popq	%rbx
	popq	%rdi
	popq	%rsi
	popq	%r14
	popq	%r15
	popq	%rbp
	retq

	.def	doNothingButTestParse;
	.scl	2;
	.type	32;
	.endef
	.globl	doNothingButTestParse
	.p2align	4, 0x90
doNothingButTestParse:
	pushq	%rbp
	pushq	%rsi
	pushq	%rax
	movq	%rsp, %rbp
	movq	%rcx, %rdx
	leaq	15(,%r8,8), %rax
	andq	$-16, %rax
	callq	__chkstk
	subq	%rax, %rsp
	movq	%rsp, %rsi
	testq	%r8, %r8
	jle	.LBB6_1
	shlq	$3, %r8
	subq	$32, %rsp
	movq	%rsi, %rcx
	callq	memcpy
	addq	$32, %rsp
	movq	16(%rsi), %rax
	movq	(%rsi), %rcx
	movq	8(%rsi), %rdx
	jmp	.LBB6_3
.LBB6_1:
.LBB6_3:
	leaq	(%rdx,%rax,2), %r8
	addq	%rcx, %r8
	addq	%rdx, %rax
	addq	%r8, %rax
	addq	$18, %rax
	leaq	8(%rbp), %rsp
	popq	%rsi
	popq	%rbp
	retq

	.def	entry;
	.scl	2;
	.type	32;
	.endef
	.globl	entry
	.p2align	4, 0x90
entry:
	pushq	%rbp
	pushq	%r15
	pushq	%r14
	pushq	%r13
	pushq	%r12
	pushq	%rsi
	pushq	%rdi
	pushq	%rbx
	pushq	%rax
	movq	%rsp, %rbp
	movq	%r8, %rsi
	movq	%rdx, %r14
	movq	%rcx, %r15
	leaq	15(,%r8,8), %rcx
	andq	$-16, %rcx
	movq	%rcx, %rax
	callq	__chkstk
	subq	%rax, %rsp
	movq	%rsp, %rdi
	movq	%rcx, %rax
	callq	__chkstk
	subq	%rax, %rsp
	movq	%rsp, %rbx
	movq	%rcx, %rax
	callq	__chkstk
	subq	%rax, %rsp
	movq	%rsp, %r13
	subq	$32, %rsp
	movq	%r15, %rcx
	callq	swap
	movq	%r15, %rcx
	movq	%r14, %rdx
	movq	%rsi, %r8
	callq	swap
	movq	%r15, %rcx
	movq	%r14, %rdx
	movq	%rsi, %r8
	callq	swap
	movq	%r15, %rcx
	movq	%r14, %rdx
	movq	%rsi, %r8
	callq	swap
	addq	$32, %rsp
	testq	%rsi, %rsi
	jle	.LBB7_1
	leaq	(,%rsi,8), %r12
	subq	$32, %rsp
	movq	%rdi, %rcx
	movq	%r15, %rdx
	movq	%r12, %r8
	callq	memcpy
	movq	%rbx, %rcx
	movq	%r14, %rdx
	movq	%r12, %r8
	callq	memcpy
	addq	$32, %rsp
	xorl	%eax, %eax
	.p2align	4, 0x90
.LBB7_3:
	movq	(%rdi,%rax,8), %rcx
	addq	(%rbx,%rax,8), %rcx
	movq	%rcx, (%r13,%rax,8)
	incq	%rax
	cmpq	%rsi, %rax
	jl	.LBB7_3
	xorl	%ecx, %ecx
	xorl	%eax, %eax
	.p2align	4, 0x90
.LBB7_5:
	addq	(%r13,%rcx,8), %rax
	incq	%rcx
	cmpq	%rsi, %rcx
	jl	.LBB7_5
	jmp	.LBB7_6
.LBB7_1:
	xorl	%eax, %eax
.LBB7_6:
	leaq	8(%rbp), %rsp
	popq	%rbx
	popq	%rdi
	popq	%rsi
	popq	%r12
	popq	%r13
	popq	%r14
	popq	%r15
	popq	%rbp
	retq

	.def	fasdfasdfsa;
	.scl	2;
	.type	32;
	.endef
	.globl	fasdfasdfsa
	.p2align	4, 0x90
fasdfasdfsa:
	xorl	%eax, %eax
	retq

