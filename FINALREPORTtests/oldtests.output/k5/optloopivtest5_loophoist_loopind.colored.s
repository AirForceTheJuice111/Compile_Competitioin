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
	mov r2, #10
	mov r1, #0
	movw r3, #0
	cmp r0, r3
	bgt main$L107
main$L108:
	mov r4, #4
main$L109:
	mov r0, r4
	bl putint
	movw r0, #10
	bl putch
	mov r0, r4
	sub sp, fp, #40
	add sp, sp, #8
	pop {r4-r10, fp, lr}
	bx lr
main$L107:
	movw r0, #3
	mul r0, r2, r0
	add r10, r0, #2
	str r10, [fp, #-40]
	ldr r9, [fp, #-40]
	mov r10, r9
	str r10, [fp, #-40]
main$L104:
	movw r0, #2
	ldr r9, [fp, #-40]
	cmp r9, r0
	bgt main$L105
main$L106:
	mov r4, r1
	b main$L109
main$L105:
	ldr r9, [fp, #-40]
	mov r0, r9
	bl putint
	ldr r9, [fp, #-40]
	sub r4, r9, #3
	movw r0, #32
	bl putch
	ldr r9, [fp, #-40]
	mov r1, r9
	mov r10, r4
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
