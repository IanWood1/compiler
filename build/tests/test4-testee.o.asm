	.text
	.file	"my compiler!!!"
	.globl	test4_forward
	.p2align	4, 0x90
	.type	test4_forward,@function
test4_forward:
	movq	%rsi, %rax
	movups	(%rdi), %xmm0
	movups	16(%rdi), %xmm1
	movups	32(%rdi), %xmm2
	movups	48(%rdi), %xmm3
	movups	64(%rdi), %xmm4
	movups	%xmm4, 64(%rsi)
	movups	%xmm3, 48(%rsi)
	movups	%xmm2, 32(%rsi)
	movups	%xmm1, 16(%rsi)
	movups	%xmm0, (%rsi)
	retq
.Lfunc_end0:
	.size	test4_forward, .Lfunc_end0-test4_forward

	.globl	test4_forward2
	.p2align	4, 0x90
	.type	test4_forward2,@function
test4_forward2:
	movq	(%rdi), %rax
	retq
.Lfunc_end1:
	.size	test4_forward2, .Lfunc_end1-test4_forward2

	.globl	test4_forward3
	.p2align	4, 0x90
	.type	test4_forward3,@function
test4_forward3:
	movq	(%rdi), %rax
	retq
.Lfunc_end2:
	.size	test4_forward3, .Lfunc_end2-test4_forward3

	.globl	test4_compare
	.p2align	4, 0x90
	.type	test4_compare,@function
test4_compare:
	pushq	%rbp
	pushq	%r15
	pushq	%r14
	pushq	%r12
	pushq	%rbx
	movq	8(%rsi), %rcx
	movq	16(%rsi), %rdx
	movq	24(%rsi), %rbp
	movq	32(%rsi), %rbx
	movq	40(%rsi), %r15
	movq	48(%rsi), %r14
	movq	56(%rsi), %r11
	movq	64(%rsi), %r10
	movq	72(%rsi), %r9
	movq	(%rdi), %rdi
	movq	(%rdi), %rax
	cmpq	(%rsi), %rax
	sete	%al
	sete	-8(%rsp)
	testb	%al, %al
	sete	%sil
	cmpq	%rcx, 8(%rdi)
	sete	%al
	sete	-8(%rsp)
	testb	%al, %al
	sete	%r8b
	cmpq	%rdx, 16(%rdi)
	sete	%al
	sete	-8(%rsp)
	testb	%al, %al
	sete	%dl
	cmpq	%rbp, 24(%rdi)
	sete	%al
	sete	-8(%rsp)
	testb	%al, %al
	sete	%bpl
	cmpq	%rbx, 32(%rdi)
	sete	%al
	sete	-8(%rsp)
	testb	%al, %al
	sete	%r12b
	cmpq	%r15, 40(%rdi)
	sete	%al
	sete	-8(%rsp)
	testb	%al, %al
	sete	%al
	cmpq	%r14, 48(%rdi)
	sete	%bl
	sete	-8(%rsp)
	testb	%bl, %bl
	sete	%r14b
	cmpq	%r11, 56(%rdi)
	sete	%cl
	sete	-8(%rsp)
	testb	%cl, %cl
	sete	%cl
	cmpq	%r10, 64(%rdi)
	sete	%bl
	sete	-8(%rsp)
	testb	%bl, %bl
	sete	%r10b
	cmpq	%r9, 72(%rdi)
	sete	-8(%rsp)
	sete	%bl
	testb	%bl, %bl
	sete	%bl
	orb	%r10b, %bl
	orb	%r14b, %cl
	orb	%bl, %cl
	orb	%r12b, %al
	orb	%bpl, %al
	orb	%cl, %al
	orb	%r8b, %dl
	orb	%sil, %dl
	orb	%al, %dl
	movzbl	%dl, %eax
	andl	$1, %eax
	popq	%rbx
	popq	%r12
	popq	%r14
	popq	%r15
	popq	%rbp
	retq
.Lfunc_end3:
	.size	test4_compare, .Lfunc_end3-test4_compare

	.globl	test4
	.p2align	4, 0x90
	.type	test4,@function
test4:
	pushq	%rbp
	pushq	%r15
	pushq	%r14
	pushq	%r13
	pushq	%r12
	pushq	%rbx
	movq	(%rdi), %r15
	movq	8(%rdi), %rax
	movq	%rax, -72(%rsp)
	movq	16(%rdi), %rax
	movq	%rax, -64(%rsp)
	movq	24(%rdi), %rax
	movq	%rax, -56(%rsp)
	movq	32(%rdi), %rax
	movq	%rax, -48(%rsp)
	movq	40(%rdi), %rax
	movq	%rax, -40(%rsp)
	movq	48(%rdi), %rax
	movq	%rax, -32(%rsp)
	movq	56(%rdi), %rax
	movq	%rax, -24(%rsp)
	movq	64(%rdi), %rax
	movq	%rax, -16(%rsp)
	movq	72(%rdi), %rax
	movq	%rax, -8(%rsp)
	movq	(%rsi), %r11
	movq	8(%rsi), %rax
	movq	16(%rsi), %rdi
	movq	24(%rsi), %r9
	movq	32(%rsi), %r8
	movq	40(%rsi), %r10
	movq	48(%rsi), %rdx
	movq	56(%rsi), %r14
	movq	64(%rsi), %rcx
	movq	72(%rsi), %rbp
	cmpq	%r11, (%r15)
	sete	-88(%rsp)
	sete	%bl
	testb	%bl, %bl
	sete	%r13b
	cmpq	%rax, 8(%r15)
	sete	-88(%rsp)
	sete	%al
	testb	%al, %al
	sete	%r12b
	cmpq	%rdi, 16(%r15)
	sete	-88(%rsp)
	sete	%al
	testb	%al, %al
	sete	%bl
	cmpq	%r9, 24(%r15)
	sete	-88(%rsp)
	sete	%al
	testb	%al, %al
	sete	%r9b
	cmpq	%r8, 32(%r15)
	sete	-88(%rsp)
	sete	%al
	testb	%al, %al
	sete	%sil
	cmpq	%r10, 40(%r15)
	sete	-88(%rsp)
	sete	%al
	testb	%al, %al
	sete	%al
	cmpq	%rdx, 48(%r15)
	sete	-88(%rsp)
	sete	%dl
	testb	%dl, %dl
	sete	%dil
	cmpq	%r14, 56(%r15)
	sete	-88(%rsp)
	sete	%dl
	testb	%dl, %dl
	sete	%dl
	cmpq	%rcx, 64(%r15)
	sete	-88(%rsp)
	sete	%cl
	testb	%cl, %cl
	sete	%r8b
	cmpq	%rbp, 72(%r15)
	sete	%cl
	testb	%cl, %cl
	sete	%cl
	orb	%r8b, %cl
	orb	%dil, %dl
	orb	%cl, %dl
	orb	%sil, %al
	orb	%r9b, %al
	orb	%dl, %al
	orb	%r12b, %bl
	orb	%r13b, %bl
	orb	%al, %bl
	movzbl	%bl, %esi
	andl	$1, %esi
	cmpq	%r15, (%r11)
	sete	%al
	sete	-80(%rsp)
	testb	%al, %al
	sete	%dil
	movq	-72(%rsp), %rax
	cmpq	%rax, 8(%r11)
	sete	%al
	sete	-80(%rsp)
	testb	%al, %al
	sete	%r8b
	movq	-64(%rsp), %rax
	cmpq	%rax, 16(%r11)
	sete	%al
	sete	-80(%rsp)
	testb	%al, %al
	sete	%bl
	movq	-56(%rsp), %rax
	cmpq	%rax, 24(%r11)
	sete	%al
	sete	-80(%rsp)
	testb	%al, %al
	sete	%r9b
	movq	-48(%rsp), %rax
	cmpq	%rax, 32(%r11)
	sete	%al
	sete	-80(%rsp)
	testb	%al, %al
	sete	%r10b
	movq	-40(%rsp), %rax
	cmpq	%rax, 40(%r11)
	sete	%cl
	sete	-80(%rsp)
	testb	%cl, %cl
	sete	%cl
	movq	-32(%rsp), %rax
	cmpq	%rax, 48(%r11)
	sete	%dl
	sete	-80(%rsp)
	testb	%dl, %dl
	sete	%bpl
	movq	-24(%rsp), %rax
	cmpq	%rax, 56(%r11)
	sete	%al
	sete	-80(%rsp)
	testb	%al, %al
	sete	%al
	movq	-16(%rsp), %rdx
	cmpq	%rdx, 64(%r11)
	sete	%dl
	sete	-80(%rsp)
	testb	%dl, %dl
	sete	%r14b
	movq	-8(%rsp), %rdx
	cmpq	%rdx, 72(%r11)
	sete	%dl
	testb	%dl, %dl
	sete	%dl
	orb	%r14b, %dl
	orb	%bpl, %al
	orb	%dl, %al
	orb	%r10b, %cl
	orb	%r9b, %cl
	orb	%al, %cl
	orb	%r8b, %bl
	orb	%dil, %bl
	orb	%cl, %bl
	movzbl	%bl, %eax
	andl	$1, %eax
	addq	%rsi, %rax
	popq	%rbx
	popq	%r12
	popq	%r13
	popq	%r14
	popq	%r15
	popq	%rbp
	retq
.Lfunc_end4:
	.size	test4, .Lfunc_end4-test4

	.section	".note.GNU-stack","",@progbits
