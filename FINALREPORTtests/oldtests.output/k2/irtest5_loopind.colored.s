.balign 4
.global main
.section .text
.arm
main:
main$L108:
	push {r4-r10, fp, lr}
	sub sp, sp, #8
	add fp, sp, #40
	mov r10, #1
	str r10, [fp, #-40]
	ldr r9, [fp, #-40]
	mov r10, r9
	str r10, [fp, #-40]
main$L105:
	movw r0, #0
	ldr r9, [fp, #-40]
	cmp r9, r0
	bgt main$L104
main$L107:
	ldr r9, [fp, #-40]
	mov r0, r9
	sub sp, fp, #40
	add sp, sp, #8
	pop {r4-r10, fp, lr}
	bx lr
main$L104:
	movw r1, #0
	movw r0, #1
	sub r0, r1, r0
	ldr r9, [fp, #-40]
	cmp r9, r0
	ble main$L107
main$L106:
	ldr r9, [fp, #-40]
	mov r0, r9
	bl putint
	ldr r9, [fp, #-40]
	sub r10, r9, #1
	str r10, [fp, #-40]
	movw r0, #10
	bl putch
	ldr r9, [fp, #-40]
	mov r10, r9
	str r10, [fp, #-40]
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
