.balign 4
.global main
.section .text
.arm
main:
main$L110:
	push {r4-r10, fp, lr}
	sub sp, sp, #20
	add fp, sp, #52
	bl getint
	mov r1, r0
	mov r0, #10
	mov r10, #0
	str r10, [fp, #-40]
	movw r2, #0
	cmp r1, r2
	bgt main$L107
main$L108:
	mov r10, #4
	str r10, [fp, #-44]
	ldr r9, [fp, #-44]
	mov r10, r9
	str r10, [fp, #-44]
main$L109:
	ldr r9, [fp, #-44]
	mov r0, r9
	bl putint
	movw r0, #10
	bl putch
	ldr r9, [fp, #-44]
	mov r0, r9
	sub sp, fp, #52
	add sp, sp, #20
	pop {r4-r10, fp, lr}
	bx lr
main$L107:
	movw r1, #3
	mul r0, r0, r1
	add r0, r0, #2
	ldr r9, [fp, #-40]
	mov r1, r9
	mov r10, r0
	str r10, [fp, #-48]
main$L104:
	movw r0, #2
	ldr r9, [fp, #-48]
	cmp r9, r0
	bgt main$L105
main$L106:
	mov r10, r1
	str r10, [fp, #-44]
	b main$L109
main$L105:
	ldr r9, [fp, #-48]
	mov r0, r9
	bl putint
	ldr r9, [fp, #-48]
	sub r10, r9, #3
	str r10, [fp, #-52]
	movw r0, #32
	bl putch
	ldr r9, [fp, #-48]
	mov r1, r9
	ldr r9, [fp, #-52]
	mov r10, r9
	str r10, [fp, #-48]
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
