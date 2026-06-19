.balign 4
.global main
.section .text
.arm
main:
main$L100:
	push {r4-r10, fp, lr}
	sub sp, sp, #4
	add fp, sp, #36
	movw r107, #16
	mov r0, r107
	bl malloc
	mov r10001, r0
	movw r108, #3
	str r108, [r10001]
	movw r109, #1
	str r109, [r10001, #4]
	movw r110, #2
	str r110, [r10001, #8]
	movw r111, #3
	str r111, [r10001, #12]
main$L101:
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
