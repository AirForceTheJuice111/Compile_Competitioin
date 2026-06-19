.balign 4
.global main
.section .text
.arm
main:
main$L105:
	push {r4-r10, fp, lr}
	sub sp, sp, #16
	add fp, sp, #48
	bl getint
	mov r4, r0
	mov r10, #0
	str r10, [fp, #-40]
	bl getint
	mov r10, r0
	str r10, [fp, #-48]
	mov r0, r4
	ldr r9, [fp, #-40]
	mov r10, r9
	str r10, [fp, #-40]
main$L102:
	movw r1, #0
	cmp r0, r1
	bgt main$L103
main$L104:
	movw r0, #10
	bl putch
	ldr r9, [fp, #-40]
	mov r0, r9
	sub sp, fp, #48
	add sp, sp, #16
	pop {r4-r10, fp, lr}
	bx lr
main$L103:
	ldr r10, [fp, #-48]
	sub r4, r0, r10
	movw r0, #8
	mul r0, r0, r4
	add r10, r0, #7
	str r10, [fp, #-44]
	ldr r9, [fp, #-44]
	mov r0, r9
	bl putint
	movw r0, #32
	bl putch
	mov r0, r4
	ldr r9, [fp, #-44]
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
