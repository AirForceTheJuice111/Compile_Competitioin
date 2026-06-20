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
	mov r1, r0
	mov r0, #10
	ldr r9, [fp, #-40]
	mov r10, r9
	str r10, [fp, #-48]
main$L102:
	movw r10, #0
	str r10, [fp, #-44]
	ldr r10, [fp, #-44]
	cmp r0, r10
	bgt main$L103
main$L104:
	ldr r9, [fp, #-48]
	mov r0, r9
	sub sp, fp, #48
	add sp, sp, #16
	pop {r4-r10, fp, lr}
	bx lr
main$L103:
	sub r0, r0, #1
	mov r10, r1
	str r10, [fp, #-48]
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
