.balign 4
.global main
.section .text
.arm
main:
main$L108:
	push {r4-r10, fp, lr}
	sub sp, sp, #4
	add fp, sp, #36
	mov r10101, #0
	movw r124, #24
	mov r0, r124
	bl malloc
	mov r10001, r0
	movw r125, #4
	mul r12005, r10101, r125
	movw r126, #5
	str r126, [r10001]
	movw r127, #1
	str r127, [r10001, #4]
	movw r128, #2
	str r128, [r10001, #8]
	movw r129, #3
	str r129, [r10001, #12]
	movw r130, #4
	str r130, [r10001, #16]
	movw r131, #5
	str r131, [r10001, #20]
	add r12003, r12005, #4
	mov r10102, r10101
	mov r12002, r12003
main$L102:
	mov r10601, r10102
	ldr r10301, [r10001]
	cmp r10601, r10301
	blt main$L103
main$L104:
	movw r133, #1
	mov r0, r133
	sub sp, fp, #36
	add sp, sp, #4
	pop {r4-r10, fp, lr}
	bx lr
main$L103:
	mov r10801, r10001
	ldr r10401, [r10001]
	movw r134, #0
	cmp r10102, r134
	bge main$L106
main$L105:
	movw r135, #65535
	movt r135, #65535
	mov r0, r135
	bl exit
main$L106:
	cmp r10102, r10401
	bge main$L105
main$L107:
	ldr r10901, [r10801, r12002]
	add r10103, r10102, #1
	add r12004, r12002, #4
	mov r0, r10901
	bl putint
	movw r132, #10
	mov r0, r132
	bl putch
	mov r10102, r10103
	mov r12002, r12004
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
