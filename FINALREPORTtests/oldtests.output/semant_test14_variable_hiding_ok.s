.balign 4
.global main
.section .text
.arm
main:
main$L100:
	push {r4-r10, fp, lr}
	sub sp, sp, #4
	add fp, sp, #36
	movw r108, #8
	mov r0, r108
	bl malloc
	mov r10201, r0
	mov r10001, r10201
	movw r109, #1
	str r109, [r10001, #4]
	ldr r10701, [r10001, #4]
	mov r0, r10701
	sub sp, fp, #36
	add sp, sp, #4
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
