.balign 4
.global main
.section .text
.arm
main:
main$L105:
	push {r4-r10, fp, lr}
	sub sp, sp, #8
	add fp, sp, #40
	mov r4, #10
	sub r0, r4, #1
	movw r1, #4
	mul r0, r0, r1
	sub r10, r0, #2
	str r10, [fp, #-40]
	ldr r9, [fp, #-40]
	mov r10, r9
	str r10, [fp, #-40]
main$L102:
	movw r0, #65530
	movt r0, #65535
	ldr r9, [fp, #-40]
	cmp r9, r0
	bgt main$L103
main$L104:
	mov r0, r4
	bl putint
	movw r0, #10
	bl putch
	mov r0, r4
	sub sp, fp, #40
	add sp, sp, #8
	pop {r4-r10, fp, lr}
	bx lr
main$L103:
	sub r4, r4, #1
	ldr r9, [fp, #-40]
	mov r0, r9
	bl putint
	ldr r9, [fp, #-40]
	sub r10, r9, #4
	str r10, [fp, #-40]
	movw r0, #32
	bl putch
	ldr r9, [fp, #-40]
	mov r10, r9
	str r10, [fp, #-40]
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
