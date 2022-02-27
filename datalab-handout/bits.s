	.file	"bits.c"
	.text
	.globl	bitXor
	.type	bitXor, @function
bitXor:
.LFB0:
	.cfi_startproc
	endbr64
	movl	%edi, %eax
	xorl	%esi, %eax
	ret
	.cfi_endproc
.LFE0:
	.size	bitXor, .-bitXor
	.globl	tmin
	.type	tmin, @function
tmin:
.LFB1:
	.cfi_startproc
	endbr64
	movl	$-2147483648, %eax
	ret
	.cfi_endproc
.LFE1:
	.size	tmin, .-tmin
	.globl	isTmax
	.type	isTmax, @function
isTmax:
.LFB2:
	.cfi_startproc
	endbr64
	cmpl	$-1, %edi
	sete	%al
	movzbl	%al, %eax
	ret
	.cfi_endproc
.LFE2:
	.size	isTmax, .-isTmax
	.globl	isTmax2
	.type	isTmax2, @function
isTmax2:
.LFB3:
	.cfi_startproc
	endbr64
	cmpl	$-1, %edi
	sete	%al
	movzbl	%al, %eax
	ret
	.cfi_endproc
.LFE3:
	.size	isTmax2, .-isTmax2
	.globl	allOddBits
	.type	allOddBits, @function
allOddBits:
.LFB4:
	.cfi_startproc
	endbr64
	orl	$1431655765, %edi
	cmpl	$-1, %edi
	sete	%al
	movzbl	%al, %eax
	ret
	.cfi_endproc
.LFE4:
	.size	allOddBits, .-allOddBits
	.globl	negate
	.type	negate, @function
negate:
.LFB5:
	.cfi_startproc
	endbr64
	movl	%edi, %eax
	negl	%eax
	ret
	.cfi_endproc
.LFE5:
	.size	negate, .-negate
	.globl	isAsciiDigit
	.type	isAsciiDigit, @function
isAsciiDigit:
.LFB6:
	.cfi_startproc
	endbr64
	movl	%edi, %eax
	orl	$7, %eax
	orl	$1, %edi
	cmpl	$55, %eax
	sete	%al
	cmpl	$57, %edi
	sete	%dl
	orl	%edx, %eax
	movzbl	%al, %eax
	ret
	.cfi_endproc
.LFE6:
	.size	isAsciiDigit, .-isAsciiDigit
	.globl	conditional
	.type	conditional, @function
conditional:
.LFB7:
	.cfi_startproc
	endbr64
	movl	%edx, %eax
	testl	%edi, %edi
	sete	%cl
	movzbl	%cl, %ecx
	movl	%ecx, %edx
	negl	%edx
	subl	$1, %ecx
	andl	%esi, %ecx
	andl	%eax, %edx
	movl	%ecx, %eax
	orl	%edx, %eax
	ret
	.cfi_endproc
.LFE7:
	.size	conditional, .-conditional
	.globl	isLessOrEqual
	.type	isLessOrEqual, @function
isLessOrEqual:
.LFB8:
	.cfi_startproc
	endbr64
	movl	%edi, %eax
	movl	%esi, %edx
	notl	%edx
	movl	%edx, %edi
	orl	%eax, %edi
	subl	%eax, %esi
	andl	%edx, %eax
	notl	%esi
	andl	%edi, %esi
	orl	%eax, %esi
	movl	%esi, %eax
	shrl	$31, %eax
	ret
	.cfi_endproc
.LFE8:
	.size	isLessOrEqual, .-isLessOrEqual
	.globl	logicalNeg
	.type	logicalNeg, @function
logicalNeg:
.LFB9:
	.cfi_startproc
	endbr64
	movl	$2, %eax
	ret
	.cfi_endproc
.LFE9:
	.size	logicalNeg, .-logicalNeg
	.globl	howManyBits
	.type	howManyBits, @function
howManyBits:
.LFB10:
	.cfi_startproc
	endbr64
	movl	$0, %eax
	ret
	.cfi_endproc
.LFE10:
	.size	howManyBits, .-howManyBits
	.globl	floatScale2
	.type	floatScale2, @function
floatScale2:
.LFB11:
	.cfi_startproc
	endbr64
	movl	$2, %eax
	ret
	.cfi_endproc
.LFE11:
	.size	floatScale2, .-floatScale2
	.globl	floatFloat2Int
	.type	floatFloat2Int, @function
floatFloat2Int:
.LFB12:
	.cfi_startproc
	endbr64
	movl	$2, %eax
	ret
	.cfi_endproc
.LFE12:
	.size	floatFloat2Int, .-floatFloat2Int
	.globl	floatPower2
	.type	floatPower2, @function
floatPower2:
.LFB13:
	.cfi_startproc
	endbr64
	movl	$2, %eax
	ret
	.cfi_endproc
.LFE13:
	.size	floatPower2, .-floatPower2
	.ident	"GCC: (Ubuntu 9.3.0-17ubuntu1~20.04) 9.3.0"
	.section	.note.GNU-stack,"",@progbits
	.section	.note.gnu.property,"a"
	.align 8
	.long	 1f - 0f
	.long	 4f - 1f
	.long	 5
0:
	.string	 "GNU"
1:
	.align 8
	.long	 0xc0000002
	.long	 3f - 2f
2:
	.long	 0x3
3:
	.align 8
4:
