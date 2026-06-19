.balign 4
.global main
.section .text
.arm
main:
main$L105:
	push {r4-r10, fp, lr}
	sub sp, sp, #8
	add fp, sp, #40
	movw r0, #4
	bl malloc
	mov r1, r0
main$L102:
	ldr r0, [r1]
	movw r10, #0
	str r10, [fp, #-40]
	ldr r10, [fp, #-40]
	cmp r0, r10
	bne main$L103
main$L104:
	movw r0, #0
	sub sp, fp, #40
	add sp, sp, #8
	pop {r4-r10, fp, lr}
	bx lr
main$L103:
	b main$L102

.global malloc
.global getint
.global getch
.global getarray
.global putint
.global putch
.global putarray
.global starttime
.global stoptime
