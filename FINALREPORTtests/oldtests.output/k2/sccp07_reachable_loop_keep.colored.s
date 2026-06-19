.balign 4
.global main
.section .text
.arm
main:
main$L105:
	push {r4-r10, fp, lr}
	sub sp, sp, #8
	add fp, sp, #40
	mov r1, #0
	mov r0, #0
main$L102:
	movw r10, #3
	str r10, [fp, #-40]
	ldr r10, [fp, #-40]
	cmp r1, r10
	blt main$L103
main$L104:
	sub sp, fp, #40
	add sp, sp, #8
	pop {r4-r10, fp, lr}
	bx lr
main$L103:
	mov r0, #5
	add r1, r1, #1
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
