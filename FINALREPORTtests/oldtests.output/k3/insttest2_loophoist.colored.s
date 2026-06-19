.balign 4
.global main
.section .text
.arm
main:
main$L108:
	push {r4-r10, fp, lr}
	sub sp, sp, #32
	add fp, sp, #64
	mov r10, #3
	str r10, [fp, #-40]
	movw r0, #4
	bl malloc
	mov r10, r0
	str r10, [fp, #-48]
	movw r0, #20
	bl malloc
	mov r10, r0
	str r10, [fp, #-52]
	movw r0, #4
	ldr r10, [fp, #-52]
	str r0, [r10]
	ldr r9, [fp, #-48]
	mov r0, r9
	ldr r9, [fp, #-48]
	mov r2, r9
	movw r1, #1
	ldr r10, [fp, #-52]
	str r1, [r10, #4]
	movw r1, #2
	ldr r10, [fp, #-52]
	str r1, [r10, #8]
	movw r1, #3
	ldr r10, [fp, #-52]
	str r1, [r10, #12]
	movw r1, #4
	ldr r10, [fp, #-52]
	str r1, [r10, #16]
	ldr r9, [fp, #-52]
	str r9, [r0]
	mov r10, r2
	str r10, [fp, #-64]
	ldr r9, [fp, #-40]
	mov r0, r9
main$L102:
	movw r1, #0
	cmp r0, r1
	bge main$L103
main$L104:
	movw r0, #10
	bl putch
	movw r0, #2
	sub sp, fp, #64
	add sp, sp, #32
	pop {r4-r10, fp, lr}
	bx lr
main$L103:
	sub r10, r0, #1
	str r10, [fp, #-44]
	ldr r9, [fp, #-64]
	ldr r10, [r9]
	str r10, [fp, #-60]
	ldr r9, [fp, #-60]
	mov r10, r9
	str r10, [fp, #-60]
	ldr r9, [fp, #-60]
	ldr r10, [r9]
	str r10, [fp, #-56]
	movw r0, #0
	ldr r9, [fp, #-44]
	cmp r9, r0
	bge main$L106
main$L105:
	movw r0, #65535
	movt r0, #65535
	bl exit
main$L106:
	ldr r9, [fp, #-44]
	ldr r10, [fp, #-56]
	cmp r9, r10
	bge main$L105
main$L107:
	ldr r9, [fp, #-44]
	add r0, r9, #1
	movw r1, #4
	mul r0, r0, r1
	ldr r9, [fp, #-60]
	ldr r0, [r9, r0]
	bl putint
	movw r0, #32
	bl putch
	ldr r9, [fp, #-44]
	mov r0, r9
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
