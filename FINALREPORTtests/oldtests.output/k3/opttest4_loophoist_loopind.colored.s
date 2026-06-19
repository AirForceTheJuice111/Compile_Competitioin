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
	mov r10, r0
	str r10, [fp, #-44]
	mov r0, #10
	mov r1, #0
	add r1, r1, #1
	movw r2, #3
	mul r1, r1, r2
	mov r10, r0
	str r10, [fp, #-52]
	ldr r9, [fp, #-40]
	mov r0, r9
	ldr r9, [fp, #-44]
	mov r2, r9
	mov r10, r1
	str r10, [fp, #-48]
main$L102:
	movw r1, #0
	ldr r9, [fp, #-52]
	cmp r9, r1
	bgt main$L103
main$L104:
	sub sp, fp, #52
	add sp, sp, #20
	pop {r4-r10, fp, lr}
	bx lr
main$L103:
main$L107:
	movw r1, #0
	cmp r2, r1
	bgt main$L108
main$L109:
	ldr r9, [fp, #-52]
	sub r10, r9, #1
	str r10, [fp, #-52]
	ldr r9, [fp, #-48]
	add r10, r9, #3
	str r10, [fp, #-48]
	ldr r9, [fp, #-52]
	mov r10, r9
	str r10, [fp, #-52]
	ldr r9, [fp, #-48]
	mov r10, r9
	str r10, [fp, #-48]
	b main$L102
main$L108:
	sub r2, r2, #1
	ldr r9, [fp, #-48]
	mov r0, r9
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
