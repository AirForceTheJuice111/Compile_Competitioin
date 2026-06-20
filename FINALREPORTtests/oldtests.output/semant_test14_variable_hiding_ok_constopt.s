.balign 4
.global main
.section .text
.arm
main:
main$L100:
	push {r4-r10, fp, lr}
	sub sp, sp, #4
	add fp, sp, #36
	movw r110, #8
	mov r0, r110
	bl malloc
	mov r1020101, r0
	mov r1000101, r1020101
	movw r111, #1
	str r111, [r1000101, #4]
	ldr r1070101, [r1000101, #4]
	mov r0, r1070101
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
