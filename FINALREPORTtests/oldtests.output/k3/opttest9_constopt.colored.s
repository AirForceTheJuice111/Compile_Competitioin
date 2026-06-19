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
	mov r2, r0
	mov r10, #10
	str r10, [fp, #-40]
	mov r0, #100
	ldr r9, [fp, #-40]
	mov r10, r9
	str r10, [fp, #-40]
main$L102:
	movw r1, #0
	ldr r9, [fp, #-40]
	cmp r9, r1
	bgt main$L103
main$L104:
	sub sp, fp, #40
	add sp, sp, #8
	pop {r4-r10, fp, lr}
	bx lr
main$L103:
	mov r1, r2
main$L107:
	movw r2, #0
	cmp r1, r2
	bgt main$L108
main$L109:
	ldr r9, [fp, #-40]
	sub r10, r9, #1
	str r10, [fp, #-40]
	ldr r9, [fp, #-40]
	mov r10, r9
	str r10, [fp, #-40]
	mov r2, r1
	b main$L102
main$L108:
	sub r1, r1, #1
	mov r0, #63
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
