.balign 4
.global main
.section .text
.arm
main:
main$L105:
	push {r4-r10, fp, lr}
	sub sp, sp, #12
	add fp, sp, #44
	mov r0, #10
	sub r1, r0, #1
	movw r2, #4
	mul r1, r1, r2
	sub r1, r1, #2
	mov r10, r0
	str r10, [fp, #-44]
	mov r10, r1
	str r10, [fp, #-40]
main$L102:
	movw r0, #65530
	movt r0, #65535
	ldr r9, [fp, #-40]
	cmp r9, r0
	bgt main$L103
main$L104:
	ldr r9, [fp, #-44]
	mov r0, r9
	bl putint
	movw r0, #10
	bl putch
	ldr r9, [fp, #-44]
	mov r0, r9
	sub sp, fp, #44
	add sp, sp, #12
	pop {r4-r10, fp, lr}
	bx lr
main$L103:
	ldr r9, [fp, #-44]
	sub r10, r9, #1
	str r10, [fp, #-44]
	ldr r9, [fp, #-40]
	mov r0, r9
	bl putint
	ldr r9, [fp, #-40]
	sub r10, r9, #4
	str r10, [fp, #-40]
	movw r0, #32
	bl putch
	ldr r9, [fp, #-44]
	mov r10, r9
	str r10, [fp, #-44]
	ldr r9, [fp, #-40]
	mov r10, r9
	str r10, [fp, #-40]
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
