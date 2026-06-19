.balign 4
.global main
.section .text
.arm
main:
main$L100:
	push {r4-r10, fp, lr}
	sub sp, sp, #4
	add fp, sp, #36
	mov r10201, #2
	add r10701, r10201, #1
	movw r110, #4
	mul r10401, r10701, r110
	mov r0, r10401
	bl malloc
	mov r10301, r0
	str r10201, [r10301]
	mov r10001, r10301
	movw r111, #1
	mov r0, r111
	mov r1, r10001
	bl putarray
	movw r112, #0
	mov r0, r112
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
