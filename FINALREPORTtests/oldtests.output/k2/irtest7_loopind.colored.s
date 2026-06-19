.balign 4
.global main
.section .text
.arm
main:
main$L108:
	push {r4-r10, fp, lr}
	sub sp, sp, #16
	add fp, sp, #48
	mov r10, #1
	str r10, [fp, #-40]
	mov r10, #2
	str r10, [fp, #-44]
	bl getint
	mov r1, r0
	mov r10, #0
	str r10, [fp, #-48]
	movw r0, #1
	ldr r9, [fp, #-40]
	cmp r9, r0
	bgt main$L105
main$L104:
	movw r0, #2
	ldr r9, [fp, #-44]
	cmp r9, r0
	blt main$L105
	ldr r9, [fp, #-48]
	mov r10, r9
	str r10, [fp, #-48]
main$L106:
	ldr r10, [fp, #-48]
	add r0, r1, r10
	sub sp, fp, #48
	add sp, sp, #16
	pop {r4-r10, fp, lr}
	bx lr
main$L105:
	mov r10, #1
	str r10, [fp, #-48]
	ldr r9, [fp, #-48]
	mov r10, r9
	str r10, [fp, #-48]
	b main$L106

.global malloc
.global getint
.global getch
.global getarray
.global putint
.global putch
.global putarray
.global starttime
.global stoptime
