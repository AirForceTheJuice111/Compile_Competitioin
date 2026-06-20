.balign 4
.global main
.section .text
.arm
main:
main$L116:
	push {r4-r10, fp, lr}
	sub sp, sp, #72
	add fp, sp, #104
	movw r10, #1030
	str r10, [fp, #-60]
	mov r10, #1000
	str r10, [fp, #-44]
	mov r3, #0
	mov r2, #0
	add r1, r3, #1
	add r0, r3, #1
	ldr r10, [fp, #-44]
	mul r1, r1, r10
	movw r4, #10
	mul r0, r0, r4
	mov r10, r2
	str r10, [fp, #-64]
	mov r2, r3
	mov r10, r1
	str r10, [fp, #-84]
	mov r10, r0
	str r10, [fp, #-92]
main$L102:
	ldr r10, [fp, #-60]
	cmp r2, r10
	blt main$L103
main$L104:
	movw r0, #10
	bl putch
	ldr r9, [fp, #-64]
	mov r0, r9
	bl putint
	movw r0, #10
	bl putch
	movw r0, #0
	sub sp, fp, #104
	add sp, sp, #72
	pop {r4-r10, fp, lr}
	bx lr
main$L103:
	add r10, r2, #1
	str r10, [fp, #-48]
	ldr r9, [fp, #-64]
	ldr r10, [fp, #-84]
	add r10, r9, r10
	str r10, [fp, #-68]
	mov r10, #0
	str r10, [fp, #-52]
	ldr r9, [fp, #-52]
	ldr r10, [fp, #-48]
	mul r10, r9, r10
	str r10, [fp, #-104]
	ldr r9, [fp, #-48]
	ldr r10, [fp, #-44]
	mul r4, r9, r10
	movw r0, #4
	ldr r9, [fp, #-52]
	mul r1, r9, r0
	add r0, r4, #1
	add r10, r1, #4
	str r10, [fp, #-100]
	movw r1, #4
	mul r0, r0, r1
	bl malloc
	str r4, [r0]
	mov r10, r0
	str r10, [fp, #-40]
	ldr r9, [fp, #-52]
	mov r10, r9
	str r10, [fp, #-56]
	ldr r9, [fp, #-100]
	mov r10, r9
	str r10, [fp, #-96]
	ldr r9, [fp, #-104]
	mov r4, r9
main$L107:
	ldr r9, [fp, #-56]
	ldr r10, [fp, #-92]
	cmp r9, r10
	blt main$L108
main$L109:
	ldr r9, [fp, #-56]
	sub r10, r9, #1
	str r10, [fp, #-76]
	ldr r9, [fp, #-40]
	mov r10, r9
	str r10, [fp, #-80]
	ldr r9, [fp, #-40]
	ldr r4, [r9]
	movw r0, #0
	ldr r9, [fp, #-76]
	cmp r9, r0
	bge main$L114
main$L113:
	movw r0, #65535
	movt r0, #65535
	bl exit
main$L114:
	ldr r9, [fp, #-76]
	cmp r9, r4
	bge main$L113
main$L115:
	ldr r9, [fp, #-76]
	add r0, r9, #1
	ldr r9, [fp, #-84]
	ldr r10, [fp, #-44]
	add r10, r9, r10
	str r10, [fp, #-88]
	ldr r9, [fp, #-92]
	add r4, r9, #10
	movw r1, #4
	mul r0, r0, r1
	ldr r9, [fp, #-80]
	ldr r0, [r9, r0]
	bl putint
	movw r0, #32
	bl putch
	ldr r9, [fp, #-68]
	mov r10, r9
	str r10, [fp, #-64]
	ldr r9, [fp, #-48]
	mov r2, r9
	ldr r9, [fp, #-88]
	mov r10, r9
	str r10, [fp, #-84]
	mov r10, r4
	str r10, [fp, #-92]
	b main$L102
main$L108:
	ldr r9, [fp, #-40]
	mov r10, r9
	str r10, [fp, #-40]
	ldr r9, [fp, #-40]
	ldr r10, [r9]
	str r10, [fp, #-72]
	movw r0, #0
	ldr r9, [fp, #-56]
	cmp r9, r0
	bge main$L111
main$L110:
	movw r0, #65535
	movt r0, #65535
	bl exit
main$L111:
	ldr r9, [fp, #-56]
	ldr r10, [fp, #-72]
	cmp r9, r10
	bge main$L110
main$L112:
	ldr r10, [fp, #-40]
	str r4, [r10, r10]
	ldr r9, [fp, #-56]
	add r10, r9, #1
	str r10, [fp, #-56]
	ldr r9, [fp, #-96]
	add r10, r9, #4
	str r10, [fp, #-96]
	ldr r10, [fp, #-48]
	add r4, r4, r10
	ldr r9, [fp, #-56]
	mov r10, r9
	str r10, [fp, #-56]
	ldr r9, [fp, #-96]
	mov r10, r9
	str r10, [fp, #-96]
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
