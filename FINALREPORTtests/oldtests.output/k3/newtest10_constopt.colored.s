.balign 4
.global main
.section .text
.arm
main:
main$L116:
	push {r4-r10, fp, lr}
	sub sp, sp, #8
	add fp, sp, #40
	movw r0, #404
	bl malloc
	movw r1, #100
	str r1, [r0]
	mov r10, r0
	str r10, [fp, #-40]
main$L102:
	ldr r9, [fp, #-40]
	ldr r0, [r9]
	movw r1, #0
	cmp r1, r0
	blt main$L103
main$L104:
	ldr r9, [fp, #-40]
	ldr r0, [r9]
	movw r1, #99
	cmp r0, r1
	bgt main$L113
main$L114:
main$L115:
	movw r0, #10
	bl putch
	movw r0, #1
	sub sp, fp, #40
	add sp, sp, #8
	pop {r4-r10, fp, lr}
	bx lr
main$L113:
	ldr r9, [fp, #-40]
	mov r10, r9
	str r10, [fp, #-40]
	ldr r9, [fp, #-40]
	ldr r0, [r9]
main$L111:
	movw r1, #99
	cmp r1, r0
	bge main$L110
main$L112:
	ldr r9, [fp, #-40]
	ldr r0, [r9, #400]
	bl putint
	b main$L115
main$L110:
	movw r0, #65535
	movt r0, #65535
	bl exit
main$L103:
	ldr r9, [fp, #-40]
	mov r10, r9
	str r10, [fp, #-40]
	ldr r9, [fp, #-40]
	ldr r0, [r9]
main$L106:
	movw r1, #0
	cmp r1, r0
	bge main$L105
main$L107:
	movw r0, #0
	ldr r10, [fp, #-40]
	str r0, [r10, #4]
	b main$L102
main$L105:
	movw r0, #65535
	movt r0, #65535
	bl exit

.global malloc
.global getint
.global getch
.global getarray
.global putint
.global putch
.global putarray
.global starttime
.global stoptime
