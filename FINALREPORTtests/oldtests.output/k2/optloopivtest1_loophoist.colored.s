.balign 4
.global main
.section .text
.arm
main:
main$L105:
	push {r4-r10, fp, lr}
	sub sp, sp, #8
	add fp, sp, #40
	bl getint
main$L102:
	movw r1, #0
	cmp r0, r1
	bgt main$L103
main$L104:
	movw r0, #10
	bl putch
	movw r0, #2
	sub sp, fp, #40
	add sp, sp, #8
	pop {r4-r10, fp, lr}
	bx lr
main$L103:
	sub r10, r0, #1
	str r10, [fp, #-40]
	movw r0, #4
	ldr r10, [fp, #-40]
	mul r0, r0, r10
	add r0, r0, #2
	bl putint
	movw r0, #32
	bl putch
	ldr r9, [fp, #-40]
	mov r0, r9
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
