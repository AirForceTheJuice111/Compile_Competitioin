.balign 4
.global main
.section .text
.arm
main:
main$L100:
	push {r4-r10, fp, lr}
	sub sp, sp, #4
	add fp, sp, #36
	movw r0, #16
	bl malloc
	mov r1, r0
	movw r0, #3
	str r0, [r1]
	movw r0, #1
	str r0, [r1, #4]
	movw r0, #2
	str r0, [r1, #8]
	movw r0, #3
	str r0, [r1, #12]
main$L101:
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
