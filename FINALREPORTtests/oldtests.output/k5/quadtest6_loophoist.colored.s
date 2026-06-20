.balign 4
.global main
.section .text
.arm
main:
main$L108:
	push {r4-r10, fp, lr}
	sub sp, sp, #12
	add fp, sp, #44
	mov r4, #0
	movw r0, #24
	bl malloc
	mov r10, r0
	str r10, [fp, #-44]
	movw r0, #5
	ldr r10, [fp, #-44]
	str r0, [r10]
	movw r0, #1
	ldr r10, [fp, #-44]
	str r0, [r10, #4]
	movw r0, #2
	ldr r10, [fp, #-44]
	str r0, [r10, #8]
	movw r0, #3
	ldr r10, [fp, #-44]
	str r0, [r10, #12]
	movw r0, #4
	ldr r10, [fp, #-44]
	str r0, [r10, #16]
	movw r0, #5
	ldr r10, [fp, #-44]
	str r0, [r10, #20]
	ldr r9, [fp, #-44]
	mov r10, r9
	str r10, [fp, #-44]
	mov r10, r4
	str r10, [fp, #-40]
main$L102:
	ldr r9, [fp, #-40]
	mov r10, r9
	str r10, [fp, #-40]
	ldr r9, [fp, #-44]
	ldr r0, [r9]
	ldr r9, [fp, #-40]
	cmp r9, r0
	blt main$L103
main$L104:
	movw r0, #1
	sub sp, fp, #44
	add sp, sp, #12
	pop {r4-r10, fp, lr}
	bx lr
main$L103:
	ldr r9, [fp, #-44]
	ldr r4, [r9]
	movw r0, #0
	ldr r9, [fp, #-40]
	cmp r9, r0
	bge main$L106
main$L105:
	movw r0, #65535
	movt r0, #65535
	bl exit
main$L106:
	ldr r9, [fp, #-40]
	cmp r9, r4
	bge main$L105
main$L107:
	ldr r9, [fp, #-40]
	add r0, r9, #1
	ldr r9, [fp, #-40]
	add r4, r9, #1
	movw r1, #4
	mul r0, r0, r1
	ldr r9, [fp, #-44]
	ldr r0, [r9, r0]
	bl putint
	movw r0, #10
	bl putch
	mov r10, r4
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
