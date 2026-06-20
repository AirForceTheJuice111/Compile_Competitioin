.balign 4
.global main
.section .text
.arm
main:
main$L108:
	push {r4-r10, fp, lr}
	sub sp, sp, #4
	add fp, sp, #36
	movw r0, #4
	bl malloc
	mov r8, #3
	mov r4, r0
	mov r5, r0
	movw r0, #20
	bl malloc
	movw r1, #4
	str r1, [r0]
	sub r1, r8, #1
	movw r2, #1
	str r2, [r0, #4]
	movw r2, #2
	str r2, [r0, #8]
	movw r2, #3
	str r2, [r0, #12]
	movw r2, #4
	str r2, [r0, #16]
	str r0, [r4]
	movw r0, #4
	mul r0, r1, r0
	add r4, r0, #4
main$L102:
	movw r0, #0
	cmp r4, r0
	bge main$L103
main$L104:
	movw r0, #10
	bl putch
	movw r0, #2
	sub sp, fp, #36
	add sp, sp, #4
	pop {r4-r10, fp, lr}
	bx lr
main$L103:
	sub r8, r8, #1
	ldr r6, [r5]
	ldr r7, [r6]
	movw r0, #0
	cmp r8, r0
	bge main$L106
main$L105:
	movw r0, #65535
	movt r0, #65535
	bl exit
main$L106:
	cmp r8, r7
	bge main$L105
main$L107:
	ldr r0, [r6, r4]
	sub r4, r4, #4
	bl putint
	movw r0, #32
	bl putch
	b main$L102

.global malloc
.global getint
.global getch
.global getarray
.global putint
.global putch
.global putarray
.global starttime
.global stoptime
