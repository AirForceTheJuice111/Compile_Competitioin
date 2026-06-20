.balign 4
.global main
.section .text
.arm
main:
main$L108:
	push {r4-r10, fp, lr}
	sub sp, sp, #24
	add fp, sp, #56
	movw r0, #20
	bl malloc
	mov r10, r0
	str r10, [fp, #-48]
	movw r0, #4
	ldr r10, [fp, #-48]
	str r0, [r10]
	movw r0, #1
	ldr r10, [fp, #-48]
	str r0, [r10, #4]
	movw r0, #2
	ldr r10, [fp, #-48]
	str r0, [r10, #8]
	movw r0, #3
	ldr r10, [fp, #-48]
	str r0, [r10, #12]
	movw r0, #4
	ldr r10, [fp, #-48]
	str r0, [r10, #16]
	ldr r9, [fp, #-48]
	ldr r1, [r9]
	ldr r9, [fp, #-48]
	mov r10, r9
	str r10, [fp, #-48]
	sub r0, r1, #1
	movw r2, #4
	mul r0, r0, r2
	add r0, r0, #4
	mov r10, r0
	str r10, [fp, #-52]
main$L102:
	movw r0, #0
	ldr r9, [fp, #-52]
	cmp r9, r0
	bgt main$L103
main$L104:
	movw r0, #10
	bl putch
	movw r0, #2
	sub sp, fp, #56
	add sp, sp, #24
	pop {r4-r10, fp, lr}
	bx lr
main$L103:
	sub r10, r1, #1
	str r10, [fp, #-40]
	ldr r9, [fp, #-48]
	ldr r10, [r9]
	str r10, [fp, #-44]
	movw r0, #0
	ldr r9, [fp, #-40]
	cmp r9, r0
	bge main$L106
main$L105:
	movw r0, #65535
	movt r0, #65535
	bl exit
main$L106:
	ldr r9, [fp, #-40]
	ldr r10, [fp, #-44]
	cmp r9, r10
	bge main$L105
main$L107:
	ldr r9, [fp, #-48]
	ldr r10, [fp, #-52]
	ldr r0, [r9, r10]
	ldr r9, [fp, #-52]
	sub r10, r9, #4
	str r10, [fp, #-56]
	bl putint
	movw r0, #32
	bl putch
	ldr r9, [fp, #-40]
	mov r1, r9
	ldr r9, [fp, #-56]
	mov r10, r9
	str r10, [fp, #-52]
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
