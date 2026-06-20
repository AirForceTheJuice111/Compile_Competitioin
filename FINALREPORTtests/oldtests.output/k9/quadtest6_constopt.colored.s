.balign 4
.global main
.section .text
.arm
main:
main$L108:
	push {r4-r10, fp, lr}
	sub sp, sp, #4
	add fp, sp, #36
	mov r4, #0
	movw r0, #24
	bl malloc
	mov r5, r0
	movw r0, #5
	str r0, [r5]
	movw r0, #1
	str r0, [r5, #4]
	movw r0, #2
	str r0, [r5, #8]
	movw r0, #3
	str r0, [r5, #12]
	movw r0, #4
	str r0, [r5, #16]
	movw r0, #5
	str r0, [r5, #20]
main$L102:
	ldr r0, [r5]
	cmp r4, r0
	blt main$L103
main$L104:
	movw r0, #1
	sub sp, fp, #36
	add sp, sp, #4
	pop {r4-r10, fp, lr}
	bx lr
main$L103:
	ldr r6, [r5]
	movw r0, #0
	cmp r4, r0
	bge main$L106
main$L105:
	movw r0, #65535
	movt r0, #65535
	bl exit
main$L106:
	cmp r4, r6
	bge main$L105
main$L107:
	add r0, r4, #1
	add r4, r4, #1
	movw r1, #4
	mul r0, r0, r1
	ldr r0, [r5, r0]
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
