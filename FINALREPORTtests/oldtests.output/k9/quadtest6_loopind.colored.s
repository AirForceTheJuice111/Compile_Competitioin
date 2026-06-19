.balign 4
.global main
.section .text
.arm
main:
main$L108:
	push {r4-r10, fp, lr}
	sub sp, sp, #4
	add fp, sp, #36
	mov r5, #0
	movw r0, #24
	bl malloc
	mov r6, r0
	movw r0, #4
	mul r0, r5, r0
	movw r1, #5
	str r1, [r6]
	movw r1, #1
	str r1, [r6, #4]
	movw r1, #2
	str r1, [r6, #8]
	movw r1, #3
	str r1, [r6, #12]
	movw r1, #4
	str r1, [r6, #16]
	movw r1, #5
	str r1, [r6, #20]
	add r4, r0, #4
main$L102:
	ldr r0, [r6]
	cmp r5, r0
	blt main$L103
main$L104:
	movw r0, #1
	sub sp, fp, #36
	add sp, sp, #4
	pop {r4-r10, fp, lr}
	bx lr
main$L103:
	ldr r7, [r6]
	movw r0, #0
	cmp r5, r0
	bge main$L106
main$L105:
	movw r0, #65535
	movt r0, #65535
	bl exit
main$L106:
	cmp r5, r7
	bge main$L105
main$L107:
	ldr r0, [r6, r4]
	add r5, r5, #1
	add r4, r4, #4
	bl putint
	movw r0, #10
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
