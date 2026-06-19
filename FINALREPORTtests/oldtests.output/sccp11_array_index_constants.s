.balign 4
.global main
.section .text
.arm
main:
main$L106:
	push {r4-r10, fp, lr}
	sub sp, sp, #4
	add fp, sp, #36
	movw r124, #20
	mov r0, r124
	bl malloc
	mov r10001, r0
	movw r125, #1
	movw r126, #1
	add r10101, r125, r126
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
	mov r10701, r10001
	ldr r10301, [r10001]
	movw r137, #0
	cmp r10101, r137
	bge main$L101
main$L100:
	movw r138, #65535
	movt r138, #65535
	mov r0, r138
	bl exit
main$L101:
	cmp r10101, r10301
	bge main$L100
main$L102:
	add r11601, r10101, #1
	mov r10901, r10001
	movw r132, #4
	mul r11701, r11601, r132
	movw r133, #7
	str r133, [r10701, r11701]
	ldr r10401, [r10001]
	movw r139, #2
	movw r140, #0
	cmp r139, r140
	bge main$L104
main$L103:
	movw r141, #65535
	movt r141, #65535
	mov r0, r141
	bl exit
main$L104:
	movw r142, #2
	cmp r142, r10401
	bge main$L103
main$L105:
	movw r134, #2
	movw r135, #1
	add r12001, r134, r135
	movw r136, #4
	mul r12101, r12001, r136
	ldr r12301, [r10901, r12101]
	mov r0, r12301
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
