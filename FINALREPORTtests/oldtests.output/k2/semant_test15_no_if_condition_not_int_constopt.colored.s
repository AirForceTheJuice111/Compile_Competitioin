.balign 4
.global main
.section .text
.arm
main:
main$L108:
	push {r4-r10, fp, lr}
	sub sp, sp, #8
	add fp, sp, #40
	movw r0, #12
	bl malloc
	movw r1, #2
	str r1, [r0]
	ldr r1, [r0]
main$L101:
	movw r10, #0
	str r10, [fp, #-40]
	ldr r9, [fp, #-40]
	cmp r9, r1
	bge main$L100
main$L102:
	ldr r0, [r0, #4]
	movw r1, #0
	cmp r0, r1
	bne main$L105
main$L106:
main$L107:
	movw r0, #0
	sub sp, fp, #40
	add sp, sp, #8
	pop {r4-r10, fp, lr}
	bx lr
main$L105:
	movw r0, #1
	sub sp, fp, #40
	add sp, sp, #8
	pop {r4-r10, fp, lr}
	bx lr
main$L100:
	movw r0, #65535
	movt r0, #65535
	bl exit

.global malloc
.global getint
.global getch
.global getarray
.global putint
.global putch
.global putarray
.global starttime
.global stoptime
