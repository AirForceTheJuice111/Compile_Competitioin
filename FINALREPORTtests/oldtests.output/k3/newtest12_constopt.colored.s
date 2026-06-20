.balign 4
.global fib$f
.section .text
.arm
fib$f:
fib$f$L108:
	push {r4-r10, fp, lr}
	sub sp, sp, #28
	add fp, sp, #60
	mov r2, r0
	movw r0, #0
	cmp r1, r0
	beq fib$f$L105
fib$f$L104:
	movw r0, #1
	cmp r1, r0
	beq fib$f$L105
fib$f$L106:
	ldr r0, [r2]
	mov r10, r2
	str r10, [fp, #-44]
	sub r10, r1, #1
	str r10, [fp, #-40]
	mov r10, r2
	str r10, [fp, #-60]
	mov r10, r2
	str r10, [fp, #-52]
	sub r10, r1, #2
	str r10, [fp, #-48]
	mov r2, r0
	ldr r9, [fp, #-44]
	mov r0, r9
	ldr r9, [fp, #-40]
	mov r1, r9
	blx r2
	mov r1, r0
	ldr r9, [fp, #-60]
	ldr r0, [r9]
	mov r10, r1
	str r10, [fp, #-56]
	mov r2, r0
	ldr r9, [fp, #-52]
	mov r0, r9
	ldr r9, [fp, #-48]
	mov r1, r9
	blx r2
	ldr r9, [fp, #-56]
	add r0, r9, r0
	sub sp, fp, #60
	add sp, sp, #28
	pop {r4-r10, fp, lr}
	bx lr
fib$f$L105:
	mov r0, r1
	sub sp, fp, #60
	add sp, sp, #28
	pop {r4-r10, fp, lr}
	bx lr

.balign 4
.global main
.section .text
.arm
main:
main$L113:
	push {r4-r10, fp, lr}
	sub sp, sp, #16
	add fp, sp, #48
	movw r0, #4
	bl malloc
	ldr r1, =fib$f
	str r1, [r0]
	mov r10, r0
	str r10, [fp, #-48]
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
	mov r10, r0
	str r10, [fp, #-44]
	movw r0, #0
	ldr r9, [fp, #-44]
	cmp r9, r0
	blt main$L105
main$L104:
	movw r0, #47
	ldr r9, [fp, #-44]
	cmp r9, r0
	bgt main$L105
main$L106:
main$L107:
	mov r1, #0
main$L110:
	ldr r10, [fp, #-44]
	cmp r1, r10
	blt main$L111
main$L112:
	movw r0, #10
	bl putch
	movw r0, #0
	sub sp, fp, #48
	add sp, sp, #16
	pop {r4-r10, fp, lr}
	bx lr
main$L111:
	ldr r9, [fp, #-48]
	ldr r2, [r9]
	add r10, r1, #1
	str r10, [fp, #-40]
	ldr r9, [fp, #-48]
	mov r0, r9
	blx r2
	bl putint
	movw r0, #32
	bl putch
	ldr r9, [fp, #-40]
	mov r1, r9
	b main$L110
main$L105:
	movw r0, #65535
	movt r0, #65535
	sub sp, fp, #48
	add sp, sp, #16
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
