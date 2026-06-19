.balign 4
.global main
.section .text
.arm
main:
main$L110:
	push {r4-r10, fp, lr}
	sub sp, sp, #8
	add fp, sp, #40
	mov r10, #1
	str r10, [fp, #-40]
	mov r1, #2
main$L102:
	movw r0, #0
	ldr r9, [fp, #-40]
	cmp r9, r0
	bne main$L103
main$L104:
main$L112:
	movw r0, #0
	sub sp, fp, #40
	add sp, sp, #8
	pop {r4-r10, fp, lr}
	bx lr
main$L103:
	movw r0, #0
	cmp r1, r0
	bne main$L107
main$L108:
	b main$L102
main$L107:
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
