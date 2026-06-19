.balign 4
.global main
.section .text
.arm
main:
main$L100:
	push {r4-r10, fp, lr}
	sub sp, sp, #8
	add fp, sp, #40
	mov r10, #1
	str r10, [fp, #-40]
	ldr r9, [fp, #-40]
	mov r0, r9
	bl putint
	movw r0, #10
	bl putch
	ldr r9, [fp, #-40]
	mov r0, r9
	sub sp, fp, #40
	add sp, sp, #8
	pop {r4-r10, fp, lr}
	bx lr

.global malloc
.global getint
.global getch
.global getarray
.global putint
.global putch
.global putarray
.global starttime
.global stoptime
