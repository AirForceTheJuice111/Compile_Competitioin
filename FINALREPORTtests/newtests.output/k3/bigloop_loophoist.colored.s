.balign 4
.global main
.section .text
.arm
main:
main$L110:
	push {r4-r10, fp, lr}
	sub sp, sp, #32
	add fp, sp, #64
	movw r0, #3000
	mov r10, #0
	str r10, [fp, #-40]
	mov r10, #0
	str r10, [fp, #-56]
	mov r1, #0
	movw r2, #2
	mul r10, r0, r2
	str r10, [fp, #-60]
	ldr r9, [fp, #-40]
	mov r0, r9
main$L102:
	ldr r10, [fp, #-60]
	cmp r1, r10
	blt main$L103
main$L104:
	bl putint
	movw r0, #10
	bl putch
	movw r0, #0
	sub sp, fp, #64
	add sp, sp, #32
	pop {r4-r10, fp, lr}
	bx lr
main$L103:
	add r10, r1, #1
	str r10, [fp, #-52]
	movw r1, #2000
	ldr r9, [fp, #-52]
	mul r10, r9, r1
	str r10, [fp, #-64]
	movw r1, #3
	ldr r9, [fp, #-52]
	mul r10, r9, r1
	str r10, [fp, #-48]
	ldr r9, [fp, #-56]
	mov r1, r9
	mov r10, r0
	str r10, [fp, #-44]
main$L107:
	ldr r10, [fp, #-64]
	cmp r1, r10
	blt main$L108
main$L109:
	ldr r9, [fp, #-52]
	mov r1, r9
	ldr r9, [fp, #-44]
	mov r0, r9
	b main$L102
main$L108:
	add r2, r1, #1
	ldr r10, [fp, #-48]
	mul r1, r2, r10
	movw r0, #17
	sdiv r0, r2, r0
	ldr r9, [fp, #-44]
	add r1, r9, r1
	add r10, r1, r0
	str r10, [fp, #-44]
	mov r1, r2
	ldr r9, [fp, #-44]
	mov r10, r9
	str r10, [fp, #-44]
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
