.balign 4
.global main
.section .text
.arm
main:
main$L100:
	push {r4-r10, fp, lr}
	sub sp, sp, #4
	add fp, sp, #36
	movw r0, #4
	bl malloc
	ldr r1, =A$f
	str r1, [r0]
	ldr r1, [r0]
	blx r1
	movw r0, #0
	sub sp, fp, #36
	add sp, sp, #4
	pop {r4-r10, fp, lr}
	bx lr

.balign 4
.global A$f
.section .text
.arm
A$f:
A$f$L100:
	push {r4-r10, fp, lr}
	sub sp, sp, #8
	add fp, sp, #40
	mov r10, #1
	str r10, [fp, #-40]
	ldr r9, [fp, #-40]
	add r0, r9, #1
	movw r1, #4
	mul r0, r0, r1
	bl malloc
	ldr r9, [fp, #-40]
	str r9, [r0]
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
