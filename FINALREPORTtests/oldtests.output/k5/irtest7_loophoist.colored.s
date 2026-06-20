.balign 4
.global main
.section .text
.arm
main:
main$L108:
	push {r4-r10, fp, lr}
	sub sp, sp, #8
	add fp, sp, #40
	mov r10, #1
	str r10, [fp, #-40]
	mov r4, #2
	bl getint
	mov r1, #0
	movw r2, #1
	ldr r9, [fp, #-40]
	cmp r9, r2
	bgt main$L105
main$L104:
	movw r2, #2
	cmp r4, r2
	blt main$L105
main$L106:
	add r0, r0, r1
	sub sp, fp, #40
	add sp, sp, #8
	pop {r4-r10, fp, lr}
	bx lr
main$L105:
	mov r1, #1
	b main$L106

.global malloc
.global getint
.global getch
.global getarray
.global putint
.global putch
.global putarray
.global starttime
.global stoptime
