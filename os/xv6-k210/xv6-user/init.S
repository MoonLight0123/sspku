	.file	"init.c"
	.option pic
	.option norelax
	.text
	.section	.text.startup,"ax",@progbits
	.align	1
	.globl	main
	.type	main, @function
main:
	addi	sp,sp,-64
	li	a2,0
	li	a1,1
	li	a0,2
	sd	ra,56(sp)
	sd	s0,48(sp)
	sd	s1,40(sp)
	sd	s2,32(sp)
	sd	s3,24(sp)
	call	dev@plt
	li	a0,0
	call	dup@plt
	li	a0,0
	call	dup@plt
	lla	s0,.LANCHOR0
	lla	s1,.LANCHOR0+256
	lla	s3,.LANCHOR1
	addi	s2,sp,12
.L3:
	call	fork@plt
	bne	a0,zero,.L2
	ld	a0,0(s0)
	mv	a1,s3
	call	exec@plt
.L2:
	mv	a0,s2
	addi	s0,s0,8
	call	wait@plt
	bne	s0,s1,.L3
	call	my_shutdown@plt
	ld	ra,56(sp)
	ld	s0,48(sp)
	ld	s1,40(sp)
	ld	s2,32(sp)
	ld	s3,24(sp)
	li	a0,0
	addi	sp,sp,64
	jr	ra
	.size	main, .-main
	.globl	name
	.section	.rodata.str1.8,"aMS",@progbits,1
	.align	3
.LC0:
	.string	"fork"
	.align	3
.LC1:
	.string	"wait"
	.align	3
.LC2:
	.string	"getpid"
	.align	3
.LC3:
	.string	"write"
	.align	3
.LC4:
	.string	"exit"
	.align	3
.LC5:
	.string	"brk"
	.align	3
.LC6:
	.string	"yield"
	.align	3
.LC7:
	.string	"execve"
	.align	3
.LC8:
	.string	"waitpid"
	.align	3
.LC9:
	.string	"gettimeofday"
	.align	3
.LC10:
	.string	"times"
	.align	3
.LC11:
	.string	"sleep"
	.align	3
.LC12:
	.string	"getppid"
	.align	3
.LC13:
	.string	"uname"
	.align	3
.LC14:
	.string	"getcwd"
	.align	3
.LC15:
	.string	"read"
	.align	3
.LC16:
	.string	"open"
	.align	3
.LC17:
	.string	"openat"
	.align	3
.LC18:
	.string	"close"
	.align	3
.LC19:
	.string	"mkdir_"
	.align	3
.LC20:
	.string	"pipe"
	.align	3
.LC21:
	.string	"dup"
	.align	3
.LC22:
	.string	"dup2"
	.align	3
.LC23:
	.string	"getdents"
	.align	3
.LC24:
	.string	"chdir"
	.align	3
.LC25:
	.string	"fstat"
	.align	3
.LC26:
	.string	"unlink"
	.align	3
.LC27:
	.string	"clone"
	.align	3
.LC28:
	.string	"mmap"
	.align	3
.LC29:
	.string	"munmap"
	.align	3
.LC30:
	.string	"mount"
	.align	3
.LC31:
	.string	"umount"
	.globl	argv
	.bss
	.align	3
	.set	.LANCHOR1,. + 0
	.type	argv, @object
	.size	argv, 8
argv:
	.zero	8
	.section	.data.rel.local,"aw"
	.align	3
	.set	.LANCHOR0,. + 0
	.type	name, @object
	.size	name, 256
name:
	.dword	.LC0
	.dword	.LC1
	.dword	.LC2
	.dword	.LC3
	.dword	.LC4
	.dword	.LC5
	.dword	.LC6
	.dword	.LC7
	.dword	.LC8
	.dword	.LC9
	.dword	.LC10
	.dword	.LC11
	.dword	.LC12
	.dword	.LC13
	.dword	.LC14
	.dword	.LC15
	.dword	.LC16
	.dword	.LC17
	.dword	.LC18
	.dword	.LC19
	.dword	.LC20
	.dword	.LC21
	.dword	.LC22
	.dword	.LC23
	.dword	.LC24
	.dword	.LC25
	.dword	.LC26
	.dword	.LC27
	.dword	.LC28
	.dword	.LC29
	.dword	.LC30
	.dword	.LC31
	.ident	"GCC: (Ubuntu 11.4.0-1ubuntu1~22.04) 11.4.0"
	.section	.note.GNU-stack,"",@progbits
