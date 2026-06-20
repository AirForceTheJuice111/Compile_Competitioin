.balign 4
.global main
.section .text
.arm
main:
main$L110:
	push {r4-r10, fp, lr}
	sub sp, sp, #32
	add fp, sp, #64
	bl getint
	mov r10, r0
	str r10, [fp, #-44]
	mov r10, #10
	str r10, [fp, #-60]
	mov r0, #0
	add r0, r0, #1
	movw r1, #3
	mul r0, r0, r1
	ldr r9, [fp, #-60]
	mov r10, r9
	str r10, [fp, #-64]
	ldr r9, [fp, #-40]
	mov r1, r9
	ldr r9, [fp, #-44]
	mov r10, r9
	str r10, [fp, #-48]
	mov r10, r0
	str r10, [fp, #-56]
main$L102:
	movw r0, #0
	ldr r9, [fp, #-64]
	cmp r9, r0
	bgt main$L103
main$L104:
	mov r0, r1
	sub sp, fp, #64
	add sp, sp, #32
	pop {r4-r10, fp, lr}
	bx lr
main$L103:
	mov r0, r1
	ldr r9, [fp, #-48]
	mov r10, r9
	str r10, [fp, #-52]
main$L107:
	movw r1, #0
	ldr r9, [fp, #-52]
	cmp r9, r1
	bgt main$L108
main$L109:
	ldr r9, [fp, #-64]
	sub r10, r9, #1
	str r10, [fp, #-64]
	ldr r9, [fp, #-56]
	add r10, r9, #3
	str r10, [fp, #-56]
	ldr r9, [fp, #-64]
	mov r10, r9
	str r10, [fp, #-64]
	mov r1, r0
	ldr r9, [fp, #-52]
	mov r10, r9
	str r10, [fp, #-48]
	ldr r9, [fp, #-56]
	mov r10, r9
	str r10, [fp, #-56]
	b main$L102
main$L108:
	ldr r9, [fp, #-52]
	sub r10, r9, #1
	str r10, [fp, #-52]
	ldr r9, [fp, #-56]
	mov r0, r9
	ldr r9, [fp, #-52]
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
