.balign 4
.global main
.section .text
.arm
main:
main$L108:
	push {r4-r10, fp, lr}
	sub sp, sp, #8
	add fp, sp, #40
	mov r10, #0
	str r10, [fp, #-40]
	movw r1, #1
	movw r0, #0
	cmp r1, r0
	bne main$L105
main$L100:
	movw r0, #10
	ldr r10, [fp, #-40]
	sdiv r0, r0, r10
	movw r1, #0
	cmp r0, r1
	bne main$L105
main$L106:
	mov r0, #4
main$L107:
	sub sp, fp, #40
	add sp, sp, #8
	pop {r4-r10, fp, lr}
	bx lr
main$L105:
	mov r0, #3
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
