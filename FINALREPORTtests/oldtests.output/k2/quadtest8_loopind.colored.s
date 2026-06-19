.balign 4
.global main
.section .text
.arm
main:
main$L100:
	push {r4-r10, fp, lr}
	sub sp, sp, #8
	add fp, sp, #40
	movw r0, #16
	bl malloc
	ldr r1, =A$f
	str r1, [r0, #8]
	movw r1, #4
	str r1, [r0]
	movw r0, #16
	bl malloc
	mov r1, r0
	ldr r0, =B$f
	str r0, [r1, #8]
	ldr r0, =B$g
	str r0, [r1, #12]
	mov r0, r1
	movw r1, #2
	str r1, [r0, #4]
	add r10, r0, #4
	str r10, [fp, #-40]
	ldr r1, [r0, #8]
	blx r1
	mov r1, r0
	ldr r9, [fp, #-40]
	ldr r0, [r9]
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
.global B$f
.section .text
.arm
B$f:
B$f$L100:
	push {r4-r10, fp, lr}
	sub sp, sp, #4
	add fp, sp, #36
	movw r0, #200
	sub sp, fp, #36
	add sp, sp, #4
	pop {r4-r10, fp, lr}
	bx lr

.balign 4
.global B$g
.section .text
.arm
B$g:
B$g$L100:
	push {r4-r10, fp, lr}
	sub sp, sp, #4
	add fp, sp, #36
	movw r0, #300
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
	sub sp, sp, #4
	add fp, sp, #36
	movw r0, #100
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
