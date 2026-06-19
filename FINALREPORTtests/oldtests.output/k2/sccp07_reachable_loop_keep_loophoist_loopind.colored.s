.balign 4
.global main
.section .text
.arm
main:
main$L105:
	push {r4-r10, fp, lr}
	sub sp, sp, #12
	add fp, sp, #44
	mov r10, #0
	str r10, [fp, #-44]
	mov r0, #0
	mov r1, #5
	ldr r9, [fp, #-44]
	mov r10, r9
	str r10, [fp, #-44]
main$L102:
	movw r10, #3
	str r10, [fp, #-40]
	ldr r9, [fp, #-44]
	ldr r10, [fp, #-40]
	cmp r9, r10
	blt main$L103
main$L104:
	sub sp, fp, #44
	add sp, sp, #12
	pop {r4-r10, fp, lr}
	bx lr
main$L103:
	ldr r9, [fp, #-44]
	add r10, r9, #1
	str r10, [fp, #-44]
	mov r0, r1
	ldr r9, [fp, #-44]
	mov r10, r9
	str r10, [fp, #-44]
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
