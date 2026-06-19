.balign 4
.global fib$f
.section .text
.arm
fib$f:
fib$f$L108:
	push {r4-r10, fp, lr}
	sub sp, sp, #4
	add fp, sp, #36
	movw r2, #0
	cmp r1, r2
	beq fib$f$L105
fib$f$L104:
	movw r2, #1
	cmp r1, r2
	beq fib$f$L105
fib$f$L106:
	ldr r2, [r0]
	sub r3, r1, #1
	mov r4, r0
	mov r5, r0
	sub r6, r1, #2
	mov r1, r3
	blx r2
	ldr r2, [r4]
	mov r4, r0
	mov r0, r5
	mov r1, r6
	blx r2
	add r0, r4, r0
	sub sp, fp, #36
	add sp, sp, #4
	pop {r4-r10, fp, lr}
	bx lr
fib$f$L105:
	mov r0, r1
	sub sp, fp, #36
	add sp, sp, #4
	pop {r4-r10, fp, lr}
	bx lr

.balign 4
.global main
.section .text
.arm
main:
main$L113:
	push {r4-r10, fp, lr}
	sub sp, sp, #4
	add fp, sp, #36
	movw r0, #4
	bl malloc
	ldr r1, =fib$f
	str r1, [r0]
	mov r4, r0
	movw r0, #69
	bl putch
	movw r0, #110
	bl putch
	movw r0, #116
	bl putch
	movw r0, #101
	bl putch
	movw r0, #114
	bl putch
	movw r0, #32
	bl putch
	movw r0, #116
	bl putch
	movw r0, #104
	bl putch
	movw r0, #101
	bl putch
	movw r0, #32
	bl putch
	movw r0, #110
	bl putch
	movw r0, #117
	bl putch
	movw r0, #109
	bl putch
	movw r0, #98
	bl putch
	movw r0, #101
	bl putch
	movw r0, #114
	bl putch
	movw r0, #32
	bl putch
	movw r0, #111
	bl putch
	movw r0, #102
	bl putch
	movw r0, #32
	bl putch
	movw r0, #116
	bl putch
	movw r0, #101
	bl putch
	movw r0, #114
	bl putch
	movw r0, #109
	bl putch
	movw r0, #58
	bl putch
	bl getint
	mov r5, r0
	movw r0, #0
	cmp r5, r0
	blt main$L105
main$L104:
	movw r0, #47
	cmp r5, r0
	bgt main$L105
main$L106:
main$L107:
	mov r1, #0
main$L110:
	cmp r1, r5
	blt main$L111
main$L112:
	movw r0, #10
	bl putch
	movw r0, #0
	sub sp, fp, #36
	add sp, sp, #4
	pop {r4-r10, fp, lr}
	bx lr
main$L111:
	ldr r2, [r4]
	add r6, r1, #1
	mov r0, r4
	blx r2
	bl putint
	movw r0, #32
	bl putch
	mov r1, r6
	b main$L110
main$L105:
	movw r0, #65535
	movt r0, #65535
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
