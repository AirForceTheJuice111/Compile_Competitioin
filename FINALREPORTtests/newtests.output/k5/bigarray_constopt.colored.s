.balign 4
.global main
.section .text
.arm
main:
main$L116:
	push {r4-r10, fp, lr}
	sub sp, sp, #36
	add fp, sp, #68
	mov r10, #0
	str r10, [fp, #-56]
	mov r10, #0
	str r10, [fp, #-48]
	ldr r9, [fp, #-56]
	mov r10, r9
	str r10, [fp, #-56]
	ldr r9, [fp, #-48]
	mov r10, r9
	str r10, [fp, #-48]
main$L102:
	movw r0, #1030
	ldr r9, [fp, #-48]
	cmp r9, r0
	blt main$L103
main$L104:
	movw r0, #10
	bl putch
	ldr r9, [fp, #-56]
	mov r0, r9
	bl putint
	movw r0, #10
	bl putch
	movw r0, #0
	sub sp, fp, #68
	add sp, sp, #36
	pop {r4-r10, fp, lr}
	bx lr
main$L103:
	ldr r9, [fp, #-48]
	add r10, r9, #1
	str r10, [fp, #-48]
	mov r10, #0
	str r10, [fp, #-40]
	movw r0, #1000
	ldr r9, [fp, #-48]
	mul r4, r9, r0
	movw r0, #1000
	ldr r9, [fp, #-48]
	mul r0, r9, r0
	add r1, r4, #1
	ldr r9, [fp, #-56]
	add r10, r9, r0
	str r10, [fp, #-60]
	movw r0, #4
	mul r0, r1, r0
	bl malloc
	str r4, [r0]
	mov r10, r0
	str r10, [fp, #-44]
	ldr r9, [fp, #-40]
	mov r10, r9
	str r10, [fp, #-52]
main$L107:
	movw r0, #10
	ldr r9, [fp, #-48]
	mul r0, r9, r0
	ldr r9, [fp, #-52]
	cmp r9, r0
	blt main$L108
main$L109:
	ldr r9, [fp, #-52]
	sub r10, r9, #1
	str r10, [fp, #-64]
	ldr r9, [fp, #-44]
	mov r4, r9
	ldr r9, [fp, #-44]
	ldr r10, [r9]
	str r10, [fp, #-68]
	movw r0, #0
	ldr r9, [fp, #-64]
	cmp r9, r0
	bge main$L114
main$L113:
	movw r0, #65535
	movt r0, #65535
	bl exit
main$L114:
	ldr r9, [fp, #-64]
	ldr r10, [fp, #-68]
	cmp r9, r10
	bge main$L113
main$L115:
	ldr r9, [fp, #-64]
	add r0, r9, #1
	movw r1, #4
	mul r0, r0, r1
	ldr r0, [r4, r0]
	bl putint
	movw r0, #32
	bl putch
	ldr r9, [fp, #-60]
	mov r10, r9
	str r10, [fp, #-56]
	ldr r9, [fp, #-48]
	mov r10, r9
	str r10, [fp, #-48]
	b main$L102
main$L108:
	ldr r9, [fp, #-44]
	mov r10, r9
	str r10, [fp, #-44]
	ldr r9, [fp, #-44]
	ldr r4, [r9]
	movw r0, #0
	ldr r9, [fp, #-52]
	cmp r9, r0
	bge main$L111
main$L110:
	movw r0, #65535
	movt r0, #65535
	bl exit
main$L111:
	ldr r9, [fp, #-52]
	cmp r9, r4
	bge main$L110
main$L112:
	ldr r9, [fp, #-52]
	add r1, r9, #1
	ldr r9, [fp, #-52]
	ldr r10, [fp, #-48]
	mul r0, r9, r10
	ldr r9, [fp, #-52]
	add r2, r9, #1
	movw r3, #4
	mul r1, r1, r3
	ldr r10, [fp, #-44]
	str r0, [r10, r1]
	mov r10, r2
	str r10, [fp, #-52]
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
