.balign 4
.global fib$f
.section .text
.arm
fib$f:
fib$f$L107:
	push {r4-r10, fp, lr}
	sub sp, sp, #4
	add fp, sp, #36
	mov r5, r0
	mov r6, r1
	movw r0, #0
	cmp r6, r0
	beq fib$f$L105
fib$f$L104:
	movw r0, #1
	cmp r6, r0
	beq fib$f$L105
fib$f$L106:
	ldr r0, [r5]
	mov r1, r5
	sub r2, r6, #1
	push {r0}
	push {r1}
	push {r2}
	pop {r1}
	pop {r0}
	pop {ip}
	blx ip
	mov r0, r0
	mov r4, r0
	ldr r0, [r5]
	mov r1, r5
	sub r2, r6, #2
	push {r0}
	push {r1}
	push {r2}
	pop {r1}
	pop {r0}
	pop {ip}
	blx ip
	mov r0, r0
	add r0, r4, r0
	mov r0, r0
	sub sp, fp, #36
	add sp, sp, #4
	pop {r4-r10, fp, lr}
	bx lr
fib$f$L105:
	mov r0, r6
	sub sp, fp, #36
	add sp, sp, #4
	pop {r4-r10, fp, lr}
	bx lr

.balign 4
.global main
.section .text
.arm
main:
main$L112:
	push {r4-r10, fp, lr}
	sub sp, sp, #4
	add fp, sp, #36
	movw r0, #4
	push {r0}
	pop {r0}
	bl malloc
	mov r1, r0
	adr r0, fib$f
	str r0, [r1]
	movw r0, #69
	push {r0}
	pop {r0}
	bl putch
	movw r8, #110
	push {r8}
	pop {r0}
	bl putch
	movw r7, #116
	push {r7}
	pop {r0}
	bl putch
	movw r6, #101
	push {r6}
	pop {r0}
	bl putch
	movw r5, #114
	push {r5}
	pop {r0}
	bl putch
	movw r4, #32
	push {r4}
	pop {r0}
	bl putch
	push {r7}
	pop {r0}
	bl putch
	movw r0, #104
	push {r0}
	pop {r0}
	bl putch
	push {r6}
	pop {r0}
	bl putch
	push {r4}
	pop {r0}
	bl putch
	push {r8}
	pop {r0}
	bl putch
	movw r0, #117
	push {r0}
	pop {r0}
	bl putch
	movw r8, #109
	push {r8}
	pop {r0}
	bl putch
	movw r0, #98
	push {r0}
	pop {r0}
	bl putch
	push {r6}
	pop {r0}
	bl putch
	push {r5}
	pop {r0}
	bl putch
	push {r4}
	pop {r0}
	bl putch
	movw r0, #111
	push {r0}
	pop {r0}
	bl putch
	movw r0, #102
	push {r0}
	pop {r0}
	bl putch
	push {r4}
	pop {r0}
	bl putch
	push {r7}
	pop {r0}
	bl putch
	push {r6}
	pop {r0}
	bl putch
	push {r5}
	pop {r0}
	bl putch
	push {r8}
	pop {r0}
	bl putch
	movw r0, #58
	push {r0}
	pop {r0}
	bl putch
	bl getint
	mov r1, r0
	movw r0, #0
	cmp r1, r0
	blt main$L105
main$L104:
	movw r0, #47
	cmp r1, r0
	bgt main$L105
main$L106:
main$L105:
	movw r0, #65535
	movt r0, #65535
	mov r0, r0
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
