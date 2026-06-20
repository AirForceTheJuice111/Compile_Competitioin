.balign 4
.global main
.section .text
.arm
main:
main$L106:
	push {r4-r10, fp, lr}
	sub sp, sp, #4
	add fp, sp, #36
	movw r122, #16
	mov r0, r122
	bl malloc
	mov r10001, r0
	movw r123, #0
	movw r124, #1
	sub r10101, r123, r124
	movw r125, #3
	str r125, [r10001]
	movw r126, #1
	str r126, [r10001, #4]
	movw r127, #2
	str r127, [r10001, #8]
	movw r128, #3
	str r128, [r10001, #12]
	mov r10701, r10001
	ldr r10301, [r10001]
	movw r134, #0
	cmp r10101, r134
	bge main$L101
main$L100:
	movw r135, #65535
	movt r135, #65535
	mov r0, r135
	bl exit
main$L101:
	cmp r10101, r10301
	bge main$L100
main$L102:
	add r11501, r10101, #1
	mov r10901, r10001
	movw r129, #4
	mul r11601, r11501, r129
	movw r130, #4
	str r130, [r10701, r11601]
	ldr r10401, [r10001]
	movw r136, #2
	movw r137, #0
	cmp r136, r137
	bge main$L104
main$L103:
	movw r138, #65535
	movt r138, #65535
	mov r0, r138
	bl exit
main$L104:
	movw r139, #2
	cmp r139, r10401
	bge main$L103
main$L105:
	movw r131, #2
	movw r132, #1
	add r11901, r131, r132
	movw r133, #4
	mul r12001, r11901, r133
	ldr r10102, [r10901, r12001]
main$L107:
	movw r140, #0
	mov r0, r140
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
