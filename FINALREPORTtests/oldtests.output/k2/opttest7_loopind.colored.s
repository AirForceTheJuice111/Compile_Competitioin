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
	mov r10, r0
	str r10, [fp, #-40]
	mov r1, #0
	ldr r9, [fp, #-40]
	mov r10, r9
	str r10, [fp, #-40]
main$L102:
	movw r0, #0
	ldr r9, [fp, #-40]
	cmp r9, r0
	bgt main$L103
main$L104:
	mov r0, r1
	sub sp, fp, #40
	add sp, sp, #8
	pop {r4-r10, fp, lr}
	bx lr
main$L103:
	movw r0, #0
	cmp r1, r0
	bne main$L107
main$L108:
main$L109:
	mov r1, #4
	ldr r9, [fp, #-40]
	sub r10, r9, #1
	str r10, [fp, #-40]
	ldr r9, [fp, #-40]
	mov r10, r9
	str r10, [fp, #-40]
	b main$L102
main$L107:
	mov r0, r1
	sub sp, fp, #40
	add sp, sp, #8
	pop {r4-r10, fp, lr}
	bx lr

.global malloc
.global getint
.global getch
.global getarray
.global putint
.global putch
.global putarray
.global starttime
.global stoptime
