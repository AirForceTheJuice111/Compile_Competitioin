.balign 4
.global main
.section .text
.arm
main:
main$L116:
	push {r4-r10, fp, lr}
	sub sp, sp, #40
	add fp, sp, #72
	mov r10, #0
	str r10, [fp, #-56]
	mov r10, #100
	str r10, [fp, #-44]
	ldr r9, [fp, #-56]
	mov r10, r9
	str r10, [fp, #-56]
	ldr r9, [fp, #-56]
	add r0, r9, #1
	ldr r9, [fp, #-56]
	ldr r10, [fp, #-56]
	mul r10, r9, r10
	str r10, [fp, #-72]
	ldr r9, [fp, #-44]
	add r1, r9, #1
	movw r2, #4
	mul r10, r0, r2
	str r10, [fp, #-64]
	movw r0, #4
	mul r0, r1, r0
	bl malloc
	ldr r9, [fp, #-44]
	str r9, [r0]
	mov r10, r0
	str r10, [fp, #-40]
	ldr r9, [fp, #-40]
	mov r10, r9
	str r10, [fp, #-40]
	ldr r9, [fp, #-40]
	ldr r10, [fp, #-64]
	add r10, r9, r10
	str r10, [fp, #-68]
main$L102:
	ldr r9, [fp, #-40]
	ldr r0, [r9]
	ldr r9, [fp, #-56]
	cmp r9, r0
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
	sub sp, fp, #72
	add sp, sp, #40
	pop {r4-r10, fp, lr}
	bx lr
main$L113:
	ldr r9, [fp, #-40]
	mov r10, r9
	str r10, [fp, #-60]
	ldr r9, [fp, #-40]
	ldr r10, [r9]
	str r10, [fp, #-52]
	movw r1, #99
	movw r0, #0
	cmp r1, r0
	bge main$L111
main$L110:
	movw r0, #65535
	movt r0, #65535
	bl exit
main$L111:
	movw r0, #99
	ldr r10, [fp, #-52]
	cmp r0, r10
	bge main$L110
main$L112:
	movw r1, #99
	movw r0, #1
	add r0, r1, r0
	movw r1, #4
	mul r0, r0, r1
	ldr r9, [fp, #-60]
	ldr r0, [r9, r0]
	bl putint
	b main$L115
main$L103:
	ldr r9, [fp, #-40]
	ldr r10, [r9]
	str r10, [fp, #-48]
	movw r0, #0
	ldr r9, [fp, #-56]
	cmp r9, r0
	bge main$L106
main$L105:
	movw r0, #65535
	movt r0, #65535
	bl exit
main$L106:
	ldr r9, [fp, #-56]
	ldr r10, [fp, #-48]
	cmp r9, r10
	bge main$L105
main$L107:
	ldr r9, [fp, #-72]
	ldr r10, [fp, #-68]
	str r9, [r10]
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
