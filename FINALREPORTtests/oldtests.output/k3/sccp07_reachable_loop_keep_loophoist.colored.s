.balign 4
.global main
.section .text
.arm
main:
main$L105:
	push {r4-r10, fp, lr}
	sub sp, sp, #8
	add fp, sp, #40
	mov r2, #0
	mov r0, #0
	mov r1, #5
main$L102:
	movw r10, #3
	str r10, [fp, #-40]
	ldr r10, [fp, #-40]
	cmp r2, r10
	blt main$L103
main$L104:
	sub sp, fp, #40
	add sp, sp, #8
	pop {r4-r10, fp, lr}
	bx lr
main$L103:
	add r2, r2, #1
	mov r0, r1
	b main$L102

.global malloc
.global getint
.global getch
.global getarray
.global putint
.global putch
.global putarray
.global starttime
.global stoptime
