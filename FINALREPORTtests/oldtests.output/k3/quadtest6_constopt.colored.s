.balign 4
.global main
.section .text
.arm
main:
main$L108:
	push {r4-r10, fp, lr}
	sub sp, sp, #24
	add fp, sp, #56
	mov r10, #0
	str r10, [fp, #-40]
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
	ldr r9, [fp, #-40]
	mov r10, r9
	str r10, [fp, #-48]
main$L102:
	ldr r9, [fp, #-48]
	mov r10, r9
	str r10, [fp, #-48]
	ldr r9, [fp, #-44]
	ldr r0, [r9]
	ldr r9, [fp, #-48]
	cmp r9, r0
	blt main$L103
main$L104:
	movw r0, #1
	sub sp, fp, #56
	add sp, sp, #24
	pop {r4-r10, fp, lr}
	bx lr
main$L103:
	ldr r9, [fp, #-44]
	mov r10, r9
	str r10, [fp, #-44]
	ldr r9, [fp, #-44]
	ldr r10, [r9]
	str r10, [fp, #-56]
	movw r0, #0
	ldr r9, [fp, #-48]
	cmp r9, r0
	bge main$L106
main$L105:
	movw r0, #65535
	movt r0, #65535
	bl exit
main$L106:
	ldr r9, [fp, #-48]
	ldr r10, [fp, #-56]
	cmp r9, r10
	bge main$L105
main$L107:
	ldr r9, [fp, #-48]
	add r0, r9, #1
	ldr r9, [fp, #-48]
	add r10, r9, #1
	str r10, [fp, #-52]
	movw r1, #4
	mul r0, r0, r1
	ldr r9, [fp, #-44]
	ldr r0, [r9, r0]
	bl putint
	movw r0, #10
	bl putch
	ldr r9, [fp, #-52]
	mov r10, r9
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
