.balign 4
.global main
.section .text
.arm
main:
main$L110:
	push {r4-r10, fp, lr}
	sub sp, sp, #60
	add fp, sp, #92
	mov r10, #0
	str r10, [fp, #-56]
	movw r0, #3000
	mov r10, #0
	str r10, [fp, #-40]
	mov r10, #0
	str r10, [fp, #-64]
	ldr r9, [fp, #-56]
	add r10, r9, #1
	str r10, [fp, #-88]
	ldr r9, [fp, #-56]
	add r10, r9, #1
	str r10, [fp, #-52]
	movw r1, #2
	mul r10, r0, r1
	str r10, [fp, #-76]
	movw r0, #2000
	ldr r9, [fp, #-88]
	mul r10, r9, r0
	str r10, [fp, #-84]
	movw r0, #3
	ldr r9, [fp, #-52]
	mul r10, r9, r0
	str r10, [fp, #-48]
	ldr r9, [fp, #-56]
	mov r1, r9
	ldr r9, [fp, #-40]
	mov r0, r9
	ldr r9, [fp, #-84]
	mov r10, r9
	str r10, [fp, #-80]
	ldr r9, [fp, #-48]
	mov r10, r9
	str r10, [fp, #-44]
main$L102:
	ldr r10, [fp, #-76]
	cmp r1, r10
	blt main$L103
main$L104:
	bl putint
	movw r0, #10
	bl putch
	movw r0, #0
	sub sp, fp, #92
	add sp, sp, #60
	pop {r4-r10, fp, lr}
	bx lr
main$L103:
	add r10, r1, #1
	str r10, [fp, #-60]
	ldr r9, [fp, #-64]
	add r1, r9, #1
	ldr r10, [fp, #-44]
	mul r1, r1, r10
	ldr r9, [fp, #-64]
	mov r10, r9
	str r10, [fp, #-68]
	mov r10, r1
	str r10, [fp, #-92]
main$L107:
	ldr r9, [fp, #-68]
	ldr r10, [fp, #-80]
	cmp r9, r10
	blt main$L108
main$L109:
	ldr r9, [fp, #-80]
	add r10, r9, #2000
	str r10, [fp, #-80]
	ldr r9, [fp, #-44]
	add r10, r9, #3
	str r10, [fp, #-44]
	ldr r9, [fp, #-60]
	mov r1, r9
	ldr r9, [fp, #-80]
	mov r10, r9
	str r10, [fp, #-80]
	ldr r9, [fp, #-44]
	mov r10, r9
	str r10, [fp, #-44]
	b main$L102
main$L108:
	ldr r9, [fp, #-68]
	add r10, r9, #1
	str r10, [fp, #-72]
	ldr r10, [fp, #-92]
	add r1, r0, r10
	ldr r9, [fp, #-92]
	ldr r10, [fp, #-44]
	add r10, r9, r10
	str r10, [fp, #-92]
	movw r0, #17
	ldr r9, [fp, #-72]
	sdiv r0, r9, r0
	add r0, r1, r0
	ldr r9, [fp, #-72]
	mov r10, r9
	str r10, [fp, #-68]
	ldr r9, [fp, #-92]
	mov r10, r9
	str r10, [fp, #-92]
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
