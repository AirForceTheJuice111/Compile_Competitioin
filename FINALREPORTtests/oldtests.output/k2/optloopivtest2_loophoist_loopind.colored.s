.balign 4
.global main
.section .text
.arm
main:
main$L105:
	push {r4-r10, fp, lr}
	sub sp, sp, #12
	add fp, sp, #44
	bl getint
	mov r1, r0
	movw r0, #4
	mul r0, r1, r0
	add r0, r0, #2
	mov r10, r1
	str r10, [fp, #-40]
	mov r10, r0
	str r10, [fp, #-44]
main$L102:
	movw r0, #2
	ldr r9, [fp, #-44]
	cmp r9, r0
	bgt main$L103
main$L104:
	ldr r9, [fp, #-40]
	mov r0, r9
	bl putint
	movw r0, #10
	bl putch
	movw r0, #2
	sub sp, fp, #44
	add sp, sp, #12
	pop {r4-r10, fp, lr}
	bx lr
main$L103:
	ldr r9, [fp, #-44]
	mov r0, r9
	bl putint
	ldr r9, [fp, #-40]
	sub r10, r9, #2
	str r10, [fp, #-40]
	ldr r9, [fp, #-44]
	sub r10, r9, #8
	str r10, [fp, #-44]
	movw r0, #32
	bl putch
	ldr r9, [fp, #-40]
	mov r10, r9
	str r10, [fp, #-40]
	ldr r9, [fp, #-44]
	mov r10, r9
	str r10, [fp, #-44]
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
