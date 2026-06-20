.balign 4
.global main
.section .text
.arm
main:
main$L110:
	push {r4-r10, fp, lr}
	sub sp, sp, #40
	add fp, sp, #72
	movw r0, #3000
	mov r10, #0
	str r10, [fp, #-44]
	mov r10, #0
	str r10, [fp, #-60]
	mov r1, #0
	movw r10, #2
	str r10, [fp, #-40]
	ldr r10, [fp, #-40]
	mul r10, r0, r10
	str r10, [fp, #-68]
	ldr r9, [fp, #-44]
	mov r0, r9
main$L102:
	ldr r10, [fp, #-68]
	cmp r1, r10
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
	add r10, r1, #1
	str r10, [fp, #-56]
	movw r1, #2000
	ldr r9, [fp, #-56]
	mul r10, r9, r1
	str r10, [fp, #-72]
	movw r1, #3
	ldr r9, [fp, #-56]
	mul r10, r9, r1
	str r10, [fp, #-52]
	ldr r9, [fp, #-60]
	mov r1, r9
	mov r10, r0
	str r10, [fp, #-48]
main$L107:
	ldr r10, [fp, #-72]
	cmp r1, r10
	blt main$L108
main$L109:
	ldr r9, [fp, #-56]
	mov r1, r9
	ldr r9, [fp, #-48]
	mov r0, r9
	b main$L102
main$L108:
	add r10, r1, #1
	str r10, [fp, #-64]
	ldr r9, [fp, #-64]
	ldr r10, [fp, #-52]
	mul r1, r9, r10
	movw r0, #17
	ldr r9, [fp, #-64]
	sdiv r0, r9, r0
	ldr r9, [fp, #-48]
	add r1, r9, r1
	add r10, r1, r0
	str r10, [fp, #-48]
	ldr r9, [fp, #-64]
	mov r1, r9
	ldr r9, [fp, #-48]
	mov r10, r9
	str r10, [fp, #-48]
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
