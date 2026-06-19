.balign 4
.global main
.section .text
.arm
main:
main$L100:
	push {r4-r10, fp, lr}
	sub sp, sp, #4
	add fp, sp, #36
	mov r4, #2
	add r0, r4, #1
	movw r1, #4
	mul r0, r0, r1
	bl malloc
	mov r1, r0
	str r4, [r1]
	movw r0, #1
	bl putarray
	movw r0, #0
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
