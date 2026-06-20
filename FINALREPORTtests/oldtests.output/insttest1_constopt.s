.balign 4
.global main
.section .text
.arm
main:
main$L108:
	push {r4-r10, fp, lr}
	sub sp, sp, #4
	add fp, sp, #36
	movw r126, #20
	mov r0, r126
	bl malloc
	mov r10001, r0
	movw r127, #4
	str r127, [r10001]
	movw r128, #1
	str r128, [r10001, #4]
	movw r129, #2
	str r129, [r10001, #8]
	movw r130, #3
	str r130, [r10001, #12]
	movw r131, #4
	str r131, [r10001, #16]
	ldr r10301, [r10001]
	mov r10101, r10301
	mov r10102, r10101
main$L102:
	movw r135, #0
	cmp r10102, r135
	bgt main$L103
main$L104:
	movw r134, #10
	mov r0, r134
	bl putch
	movw r136, #2
	mov r0, r136
	sub sp, fp, #36
	add sp, sp, #4
	pop {r4-r10, fp, lr}
	bx lr
main$L103:
	sub r10103, r10102, #1
	mov r10701, r10001
	ldr r10401, [r10001]
	movw r137, #0
	cmp r10103, r137
	bge main$L106
main$L105:
	movw r138, #65535
	movt r138, #65535
	mov r0, r138
	bl exit
main$L106:
	cmp r10103, r10401
	bge main$L105
main$L107:
	add r11801, r10103, #1
	movw r132, #4
	mul r11901, r11801, r132
	ldr r10801, [r10701, r11901]
	mov r0, r10801
	bl putint
	movw r133, #32
	mov r0, r133
	bl putch
	mov r10102, r10103
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
