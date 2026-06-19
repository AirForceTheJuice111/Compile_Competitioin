.balign 4
.global main
.section .text
.arm
main:
main$L110:
	push {r4-r10, fp, lr}
	sub sp, sp, #8
	add fp, sp, #40
	bl getint
	mov r4, #10
	mov r10, #0
	str r10, [fp, #-40]
	movw r1, #0
	cmp r0, r1
	bgt main$L107
main$L108:
	mov r10, #4
	str r10, [fp, #-40]
	ldr r9, [fp, #-40]
	mov r10, r9
	str r10, [fp, #-40]
main$L109:
	ldr r9, [fp, #-40]
	mov r0, r9
	bl putint
	movw r0, #10
	bl putch
	ldr r9, [fp, #-40]
	mov r0, r9
	sub sp, fp, #40
	add sp, sp, #8
	pop {r4-r10, fp, lr}
	bx lr
main$L107:
	ldr r9, [fp, #-40]
	mov r10, r9
	str r10, [fp, #-40]
main$L104:
	movw r0, #0
	cmp r4, r0
	bgt main$L105
main$L106:
	ldr r9, [fp, #-40]
	mov r10, r9
	str r10, [fp, #-40]
	b main$L109
main$L105:
	movw r0, #3
	mul r0, r0, r4
	sub r4, r4, #1
	add r10, r0, #2
	str r10, [fp, #-40]
	ldr r9, [fp, #-40]
	mov r0, r9
	bl putint
	movw r0, #32
	bl putch
	ldr r9, [fp, #-40]
	mov r10, r9
	str r10, [fp, #-40]
	b main$L104

.global malloc
.global getint
.global getch
.global getarray
.global putint
.global putch
.global putarray
.global starttime
.global stoptime
