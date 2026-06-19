.balign 4
.global main
.section .text
.arm
main:
main$L105:
	push {r4-r10, fp, lr}
	sub sp, sp, #8
	add fp, sp, #40
	movw r1, #2
	movw r0, #3
	add r0, r1, r0
	movw r10, #1
	str r10, [fp, #-40]
	movw r1, #0
	ldr r9, [fp, #-40]
	cmp r9, r1
	bne main$L102
main$L103:
	bl getint
main$L104:
	sub sp, fp, #40
	add sp, sp, #8
	pop {r4-r10, fp, lr}
	bx lr
main$L102:
	mul r0, r0, r0
	b main$L104

.global malloc
.global getint
.global getch
.global getarray
.global putint
.global putch
.global putarray
.global starttime
.global stoptime
