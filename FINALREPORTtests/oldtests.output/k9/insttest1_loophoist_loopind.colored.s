.balign 4
.global main
.section .text
.arm
main:
main$L108:
	push {r4-r10, fp, lr}
	sub sp, sp, #4
	add fp, sp, #36
	movw r0, #20
	bl malloc
	mov r5, r0
	movw r0, #4
	str r0, [r5]
	movw r0, #1
	str r0, [r5, #4]
	movw r0, #2
	str r0, [r5, #8]
	movw r0, #3
	str r0, [r5, #12]
	movw r0, #4
	str r0, [r5, #16]
	ldr r6, [r5]
	sub r0, r6, #1
	movw r1, #4
	mul r0, r0, r1
	add r4, r0, #4
main$L102:
	movw r0, #0
	cmp r4, r0
	bgt main$L103
main$L104:
	movw r0, #10
	bl putch
	movw r0, #2
	sub sp, fp, #36
	add sp, sp, #4
	pop {r4-r10, fp, lr}
	bx lr
main$L103:
	sub r6, r6, #1
	ldr r7, [r5]
	movw r0, #0
	cmp r6, r0
	bge main$L106
main$L105:
	movw r0, #65535
	movt r0, #65535
	bl exit
main$L106:
	cmp r6, r7
	bge main$L105
main$L107:
	ldr r0, [r5, r4]
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
