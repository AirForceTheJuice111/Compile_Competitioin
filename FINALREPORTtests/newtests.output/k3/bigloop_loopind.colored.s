.balign 4
.global main
.section .text
.arm
main:
main$L110:
	push {r4-r10, fp, lr}
	sub sp, sp, #40
	add fp, sp, #72
	mov r10, #0
	str r10, [fp, #-48]
	movw r10, #3000
	str r10, [fp, #-60]
	mov r10, #0
	str r10, [fp, #-40]
	ldr r9, [fp, #-48]
	add r0, r9, #1
	ldr r9, [fp, #-48]
	add r2, r9, #1
	movw r1, #2000
	mul r1, r0, r1
	movw r0, #3
	mul r2, r2, r0
	ldr r9, [fp, #-48]
	mov r10, r9
	str r10, [fp, #-52]
	ldr r9, [fp, #-40]
	mov r0, r9
	mov r10, r1
	str r10, [fp, #-64]
	mov r10, r2
	str r10, [fp, #-44]
main$L102:
	movw r1, #2
	ldr r9, [fp, #-60]
	mul r1, r9, r1
	ldr r9, [fp, #-52]
	cmp r9, r1
	blt main$L103
main$L104:
	bl putint
	movw r0, #10
	bl putch
	movw r0, #0
	sub sp, fp, #72
	add sp, sp, #40
	pop {r4-r10, fp, lr}
	bx lr
main$L103:
	ldr r9, [fp, #-52]
	add r10, r9, #1
	str r10, [fp, #-56]
	mov r2, #0
	add r1, r2, #1
	ldr r10, [fp, #-44]
	mul r10, r1, r10
	str r10, [fp, #-68]
	mov r1, r2
	ldr r9, [fp, #-68]
	mov r10, r9
	str r10, [fp, #-72]
main$L107:
	ldr r10, [fp, #-64]
	cmp r1, r10
	blt main$L108
main$L109:
	ldr r9, [fp, #-64]
	add r10, r9, #2000
	str r10, [fp, #-64]
	ldr r9, [fp, #-44]
	add r10, r9, #3
	str r10, [fp, #-44]
	ldr r9, [fp, #-56]
	mov r10, r9
	str r10, [fp, #-52]
	ldr r9, [fp, #-64]
	mov r10, r9
	str r10, [fp, #-64]
	ldr r9, [fp, #-44]
	mov r10, r9
	str r10, [fp, #-44]
	b main$L102
main$L108:
	add r2, r1, #1
	ldr r10, [fp, #-72]
	add r1, r0, r10
	ldr r9, [fp, #-72]
	ldr r10, [fp, #-44]
	add r10, r9, r10
	str r10, [fp, #-72]
	movw r0, #17
	sdiv r0, r2, r0
	add r0, r1, r0
	mov r1, r2
	ldr r9, [fp, #-72]
	mov r10, r9
	str r10, [fp, #-72]
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
