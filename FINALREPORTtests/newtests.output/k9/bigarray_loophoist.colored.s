.balign 4
.global main
.section .text
.arm
main:
main$L116:
	push {r4-r10, fp, lr}
	sub sp, sp, #20
	add fp, sp, #52
	movw r10, #1030
	str r10, [fp, #-52]
	mov r10, #1000
	str r10, [fp, #-40]
	mov r10, #0
	str r10, [fp, #-44]
	mov r6, #0
	mov r10, #0
	str r10, [fp, #-48]
	ldr r9, [fp, #-44]
	mov r10, r9
	str r10, [fp, #-44]
main$L102:
	ldr r9, [fp, #-44]
	ldr r10, [fp, #-52]
	cmp r9, r10
	blt main$L103
main$L104:
	movw r0, #10
	bl putch
	mov r0, r6
	bl putint
	movw r0, #10
	bl putch
	movw r0, #0
	sub sp, fp, #52
	add sp, sp, #20
	pop {r4-r10, fp, lr}
	bx lr
main$L103:
	ldr r9, [fp, #-44]
	add r10, r9, #1
	str r10, [fp, #-44]
	ldr r9, [fp, #-44]
	ldr r10, [fp, #-40]
	mul r5, r9, r10
	ldr r9, [fp, #-44]
	ldr r10, [fp, #-40]
	mul r1, r9, r10
	movw r0, #10
	ldr r9, [fp, #-44]
	mul r4, r9, r0
	add r0, r5, #1
	add r8, r6, r1
	movw r1, #4
	mul r0, r0, r1
	bl malloc
	str r5, [r0]
	mov r7, r0
	ldr r9, [fp, #-48]
	mov r6, r9
main$L107:
	cmp r6, r4
	blt main$L108
main$L109:
	sub r5, r6, #1
	mov r4, r7
	ldr r6, [r7]
	movw r0, #0
	cmp r5, r0
	bge main$L114
main$L113:
	movw r0, #65535
	movt r0, #65535
	bl exit
main$L114:
	cmp r5, r6
	bge main$L113
main$L115:
	add r0, r5, #1
	movw r1, #4
	mul r0, r0, r1
	ldr r0, [r4, r0]
	bl putint
	movw r0, #32
	bl putch
	mov r6, r8
	ldr r9, [fp, #-44]
	mov r10, r9
	str r10, [fp, #-44]
	b main$L102
main$L108:
	ldr r5, [r7]
	movw r0, #0
	cmp r6, r0
	bge main$L111
main$L110:
	movw r0, #65535
	movt r0, #65535
	bl exit
main$L111:
	cmp r6, r5
	bge main$L110
main$L112:
	add r1, r6, #1
	ldr r10, [fp, #-44]
	mul r0, r6, r10
	add r2, r6, #1
	movw r3, #4
	mul r1, r1, r3
	str r0, [r7, r1]
	mov r6, r2
	b main$L107

.global malloc
.global getint
.global getch
.global getarray
.global putint
.global putch
.global putarray
.global starttime
.global stoptime
