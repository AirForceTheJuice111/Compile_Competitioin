.balign 4
.global main
.section .text
.arm
main:
main$L103:
	push {r4-r10, fp, lr}
	sub sp, sp, #8
	add fp, sp, #40
	movw r0, #8
	bl malloc
	movw r1, #1
	str r1, [r0]
	mov r10, r0
	str r10, [fp, #-40]
	ldr r9, [fp, #-40]
	ldr r0, [r9]
	ldr r9, [fp, #-40]
	mov r10, r9
	str r10, [fp, #-40]
main$L101:
	movw r1, #0
	cmp r1, r0
	bge main$L100
main$L102:
	movw r0, #2
	ldr r10, [fp, #-40]
	str r0, [r10, #4]
	movw r0, #0
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
