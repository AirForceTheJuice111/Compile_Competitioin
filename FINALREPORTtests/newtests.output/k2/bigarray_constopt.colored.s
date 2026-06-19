.balign 4
.global main
.section .text
.arm
main:
main$L116:
	push {r4-r10, fp, lr}
	sub sp, sp, #56
	add fp, sp, #88
	mov r10, #0
	str r10, [fp, #-60]
	mov r0, #0
	ldr r9, [fp, #-60]
	mov r10, r9
	str r10, [fp, #-60]
main$L102:
	movw r1, #1030
	cmp r0, r1
	blt main$L103
main$L104:
	movw r0, #10
	bl putch
	ldr r9, [fp, #-60]
	mov r0, r9
	bl putint
	movw r0, #10
	bl putch
	movw r0, #0
	sub sp, fp, #88
	add sp, sp, #56
	pop {r4-r10, fp, lr}
	bx lr
main$L103:
	add r10, r0, #1
	str r10, [fp, #-48]
	mov r10, #0
	str r10, [fp, #-40]
	movw r0, #1000
	ldr r9, [fp, #-48]
	mul r10, r9, r0
	str r10, [fp, #-68]
	movw r0, #1000
	ldr r9, [fp, #-48]
	mul r0, r9, r0
	ldr r9, [fp, #-68]
	add r1, r9, #1
	ldr r9, [fp, #-60]
	add r10, r9, r0
	str r10, [fp, #-64]
	movw r0, #4
	mul r0, r1, r0
	bl malloc
	ldr r9, [fp, #-68]
	str r9, [r0]
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
	str r10, [fp, #-76]
	ldr r9, [fp, #-44]
	mov r10, r9
	str r10, [fp, #-84]
	ldr r9, [fp, #-44]
	ldr r10, [r9]
	str r10, [fp, #-80]
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
	ldr r10, [fp, #-80]
	cmp r9, r10
	bge main$L113
main$L115:
	ldr r9, [fp, #-76]
	add r0, r9, #1
	movw r1, #4
	mul r0, r0, r1
	ldr r9, [fp, #-84]
	ldr r0, [r9, r0]
	bl putint
	movw r0, #32
	bl putch
	ldr r9, [fp, #-64]
	mov r10, r9
	str r10, [fp, #-60]
	ldr r9, [fp, #-48]
	mov r0, r9
	b main$L102
main$L108:
	ldr r9, [fp, #-44]
	mov r10, r9
	str r10, [fp, #-44]
	ldr r9, [fp, #-44]
	ldr r10, [r9]
	str r10, [fp, #-72]
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
	ldr r10, [fp, #-72]
	cmp r9, r10
	bge main$L110
main$L112:
	ldr r9, [fp, #-52]
	add r0, r9, #1
	ldr r9, [fp, #-52]
	ldr r10, [fp, #-48]
	mul r10, r9, r10
	str r10, [fp, #-88]
	ldr r9, [fp, #-52]
	add r10, r9, #1
	str r10, [fp, #-56]
	movw r1, #4
	mul r0, r0, r1
	ldr r9, [fp, #-88]
	ldr r10, [fp, #-44]
	str r9, [r10, r0]
	ldr r9, [fp, #-56]
	mov r10, r9
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
