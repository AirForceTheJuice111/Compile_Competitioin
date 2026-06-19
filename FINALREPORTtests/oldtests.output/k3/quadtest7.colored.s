.balign 4
.global B$f
.section .text
.arm
B$f:
B$f$L100:
	push {r4-r10, fp, lr}
	sub sp, sp, #4
	add fp, sp, #36
B$f$L101:
	movw r0, #0
	sub sp, fp, #36
	add sp, sp, #4
	pop {r4-r10, fp, lr}
	bx lr

.balign 4
.global main
.section .text
.arm
main:
main$L100:
	push {r4-r10, fp, lr}
	sub sp, sp, #8
	add fp, sp, #40
	movw r0, #12
	bl malloc
	ldr r1, =A$f
	str r1, [r0, #8]
	movw r1, #1
	str r1, [r0]
	mov r10, r0
	str r10, [fp, #-40]
	movw r0, #12
	bl malloc
	ldr r1, =B$f
	str r1, [r0, #8]
	movw r1, #10
	str r1, [r0, #4]
	add r0, r0, #4
	ldr r9, [fp, #-40]
	ldr r1, [r9]
	ldr r0, [r0]
	add r0, r1, r0
	bl putint
	movw r0, #10
	bl putch
	movw r0, #0
	sub sp, fp, #40
	add sp, sp, #8
	pop {r4-r10, fp, lr}
	bx lr

.balign 4
.global A$f
.section .text
.arm
A$f:
A$f$L100:
	push {r4-r10, fp, lr}
	sub sp, sp, #4
	add fp, sp, #36
A$f$L101:
	movw r0, #0
	sub sp, fp, #36
	add sp, sp, #4
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
