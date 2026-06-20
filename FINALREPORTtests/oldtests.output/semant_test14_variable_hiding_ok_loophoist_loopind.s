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
	mov r102010101, r0
	mov r100010101, r102010101
	movw r109, #1
	str r109, [r100010101, #4]
	ldr r107010101, [r100010101, #4]
	mov r0, r107010101
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
