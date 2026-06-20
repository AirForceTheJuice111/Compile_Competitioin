.balign 4
.global main
.section .text
.arm
main:
main$L108:
	push {r4-r10, fp, lr}
	sub sp, sp, #4
	add fp, sp, #36
	movw r124, #20
	mov r0, r124
	bl malloc
	mov r10001, r0
	movw r125, #4
	str r125, [r10001]
	movw r126, #1
	str r126, [r10001, #4]
	movw r127, #2
	str r127, [r10001, #8]
	movw r128, #3
	str r128, [r10001, #12]
	movw r129, #4
	str r129, [r10001, #16]
	ldr r10301, [r10001]
	mov r10701, r10001
	mov r10101, r10301
	sub r11905, r10101, #1
	movw r130, #4
	mul r11906, r11905, r130
	add r11903, r11906, #4
	mov r10102, r10101
	mov r11902, r11903
main$L102:
	movw r133, #0
	cmp r11902, r133
	bgt main$L103
main$L104:
	movw r132, #10
	mov r0, r132
	bl putch
	movw r134, #2
	mov r0, r134
	sub sp, fp, #36
	add sp, sp, #4
	pop {r4-r10, fp, lr}
	bx lr
main$L103:
	sub r10103, r10102, #1
	ldr r10401, [r10001]
	movw r135, #0
	cmp r10103, r135
	bge main$L106
main$L105:
	movw r136, #65535
	movt r136, #65535
	mov r0, r136
	bl exit
main$L106:
	cmp r10103, r10401
	bge main$L105
main$L107:
	ldr r10801, [r10701, r11902]
	sub r11904, r11902, #4
	mov r0, r10801
	bl putint
	movw r131, #32
	mov r0, r131
	bl putch
	mov r10102, r10103
	mov r11902, r11904
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
