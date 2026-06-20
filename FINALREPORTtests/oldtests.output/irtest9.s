.balign 4
.global main
.section .text
.arm
main:
main$L100:
	push {r4-r10, fp, lr}
	sub sp, sp, #4
	add fp, sp, #36
	mov r10201, #3
	add r10601, r10201, #1
	movw r108, #4
	mul r10401, r10601, r108
	mov r0, r10401
	bl malloc
	mov r10301, r0
	str r10201, [r10301]
	mov r10001, r10301
main$L101:
	movw r109, #0
	mov r0, r109
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
