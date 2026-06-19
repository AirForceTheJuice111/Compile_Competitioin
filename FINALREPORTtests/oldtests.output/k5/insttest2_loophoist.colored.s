.balign 4
.global main
.section .text
.arm
main:
main$L108:
	push {r4-r10, fp, lr}
	sub sp, sp, #20
	add fp, sp, #52
	mov r10, #3
	str r10, [fp, #-40]
	movw r0, #4
	bl malloc
	mov r4, r0
	movw r0, #20
	bl malloc
	mov r1, r0
	movw r0, #4
	str r0, [r1]
	mov r0, r4
	mov r2, r4
	movw r3, #1
	str r3, [r1, #4]
	movw r3, #2
	str r3, [r1, #8]
	movw r3, #3
	str r3, [r1, #12]
	movw r3, #4
	str r3, [r1, #16]
	str r1, [r0]
	mov r10, r2
	str r10, [fp, #-52]
	ldr r9, [fp, #-40]
	mov r10, r9
	str r10, [fp, #-40]
main$L102:
	movw r0, #0
	ldr r9, [fp, #-40]
	cmp r9, r0
	bge main$L103
main$L104:
	movw r0, #10
	bl putch
	movw r0, #2
	sub sp, fp, #52
	add sp, sp, #20
	pop {r4-r10, fp, lr}
	bx lr
main$L103:
	ldr r9, [fp, #-40]
	sub r10, r9, #1
	str r10, [fp, #-44]
	ldr r9, [fp, #-52]
	ldr r4, [r9]
	ldr r10, [r4]
	str r10, [fp, #-48]
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
	ldr r10, [fp, #-48]
	cmp r9, r10
	bge main$L105
main$L107:
	ldr r9, [fp, #-44]
	add r0, r9, #1
	movw r1, #4
	mul r0, r0, r1
	ldr r0, [r4, r0]
	bl putint
	movw r0, #32
	bl putch
	ldr r9, [fp, #-44]
	mov r10, r9
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
