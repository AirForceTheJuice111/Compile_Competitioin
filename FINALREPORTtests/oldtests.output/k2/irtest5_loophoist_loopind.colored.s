.balign 4
.global main
.section .text
.arm
main:
main$L108:
	push {r4-r10, fp, lr}
	sub sp, sp, #16
	add fp, sp, #48
	mov r0, #1
	movw r10, #0
	str r10, [fp, #-40]
	movw r1, #1
	ldr r9, [fp, #-40]
	sub r10, r9, r1
	str r10, [fp, #-48]
	mov r10, r0
	str r10, [fp, #-44]
main$L105:
	movw r0, #0
	ldr r9, [fp, #-44]
	cmp r9, r0
	bgt main$L104
main$L107:
	ldr r9, [fp, #-44]
	mov r0, r9
	sub sp, fp, #48
	add sp, sp, #16
	pop {r4-r10, fp, lr}
	bx lr
main$L104:
	ldr r9, [fp, #-44]
	ldr r10, [fp, #-48]
	cmp r9, r10
	ble main$L107
main$L106:
	ldr r9, [fp, #-44]
	mov r0, r9
	bl putint
	ldr r9, [fp, #-44]
	sub r10, r9, #1
	str r10, [fp, #-44]
	movw r0, #10
	bl putch
	ldr r9, [fp, #-44]
	mov r10, r9
	str r10, [fp, #-44]
	b main$L105

.global malloc
.global getint
.global getch
.global getarray
.global putint
.global putch
.global putarray
.global starttime
.global stoptime
