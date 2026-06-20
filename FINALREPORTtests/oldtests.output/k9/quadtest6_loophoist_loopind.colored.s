.balign 4
.global main
.section .text
.arm
main:
main$L108:
	push {r4-r10, fp, lr}
	sub sp, sp, #4
	add fp, sp, #36
	movw r0, #24
	bl malloc
	mov r5, r0
	mov r6, #0
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
	movw r0, #4
	mul r0, r6, r0
	add r4, r0, #4
main$L102:
	ldr r0, [r5]
	cmp r6, r0
	blt main$L103
main$L104:
	movw r0, #1
	sub sp, fp, #36
	add sp, sp, #4
	pop {r4-r10, fp, lr}
	bx lr
main$L103:
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
	add r6, r6, #1
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
