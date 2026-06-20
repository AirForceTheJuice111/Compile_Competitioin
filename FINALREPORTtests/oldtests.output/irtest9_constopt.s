.balign 4
.global main
.section .text
.arm
main:
main$L100:
	push {r4-r10, fp, lr}
	sub sp, sp, #4
	add fp, sp, #36
	movw r110, #16
	mov r0, r110
	bl malloc
	mov r1030101, r0
	movw r111, #3
	str r111, [r1030101]
	mov r1000101, r1030101
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
