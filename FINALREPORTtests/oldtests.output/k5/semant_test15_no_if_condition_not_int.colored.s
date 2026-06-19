.balign 4
.global main
.section .text
.arm
main:
main$L108:
	push {r4-r10, fp, lr}
	sub sp, sp, #8
	add fp, sp, #40
	mov r4, #2
	add r0, r4, #1
	movw r1, #4
	mul r0, r0, r1
	bl malloc
	str r4, [r0]
	ldr r10, [r0]
	str r10, [fp, #-40]
	mov r4, r0
	movw r1, #0
	movw r0, #0
	cmp r1, r0
	bge main$L101
main$L100:
	movw r0, #65535
	movt r0, #65535
	bl exit
main$L101:
	movw r0, #0
	ldr r10, [fp, #-40]
	cmp r0, r10
	bge main$L100
main$L102:
	movw r1, #0
	movw r0, #1
	add r0, r1, r0
	movw r1, #4
	mul r0, r0, r1
	ldr r0, [r4, r0]
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

.global malloc
.global getint
.global getch
.global getarray
.global putint
.global putch
.global putarray
.global starttime
.global stoptime
