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
	mov r10, #10
	str r10, [fp, #-52]
	ldr r9, [fp, #-52]
	mov r10, r9
	str r10, [fp, #-52]
	ldr r9, [fp, #-40]
	mov r10, r9
	str r10, [fp, #-44]
main$L102:
	movw r1, #0
	ldr r9, [fp, #-52]
	cmp r9, r1
	bgt main$L103
main$L104:
	ldr r9, [fp, #-44]
	mov r0, r9
	sub sp, fp, #52
	add sp, sp, #20
	pop {r4-r10, fp, lr}
	bx lr
main$L103:
	ldr r9, [fp, #-44]
	mov r10, r9
	str r10, [fp, #-48]
main$L107:
	movw r1, #0
	cmp r0, r1
	bgt main$L108
main$L109:
	ldr r9, [fp, #-52]
	sub r10, r9, #1
	str r10, [fp, #-52]
	ldr r9, [fp, #-52]
	mov r10, r9
	str r10, [fp, #-52]
	ldr r9, [fp, #-48]
	mov r10, r9
	str r10, [fp, #-44]
	b main$L102
main$L108:
	mov r10, #3
	str r10, [fp, #-48]
	sub r0, r0, #1
	ldr r9, [fp, #-48]
	mov r10, r9
	str r10, [fp, #-48]
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
