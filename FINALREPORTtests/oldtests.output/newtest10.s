.balign 4
.global main
.section .text
.arm
main:
main$L116:
	push {r4-r10, fp, lr}
	sub sp, sp, #4
	add fp, sp, #36
	mov r10001, #0
	mov r10301, #100
	add r11901, r10301, #1
	movw r132, #4
	mul r10901, r11901, r132
	mov r0, r10901
	bl malloc
	mov r10401, r0
	str r10301, [r10401]
	mov r10101, r10401
main$L102:
	mov r11101, r10001
	ldr r10501, [r10101]
	cmp r11101, r10501
	blt main$L103
main$L104:
	ldr r10701, [r10101]
	movw r138, #99
	cmp r10701, r138
	bgt main$L113
main$L114:
main$L115:
	movw r137, #10
	mov r0, r137
	bl putch
	movw r139, #1
	mov r0, r139
	sub sp, fp, #36
	add sp, sp, #4
	pop {r4-r10, fp, lr}
	bx lr
main$L113:
	mov r11501, r10101
	ldr r10801, [r10101]
	movw r140, #99
	movw r141, #0
	cmp r140, r141
	bge main$L111
main$L110:
	movw r142, #65535
	movt r142, #65535
	mov r0, r142
	bl exit
main$L111:
	movw r143, #99
	cmp r143, r10801
	bge main$L110
main$L112:
	movw r134, #99
	movw r135, #1
	add r12701, r134, r135
	movw r136, #4
	mul r12801, r12701, r136
	ldr r11601, [r11501, r12801]
	mov r0, r11601
	bl putint
	b main$L115
main$L103:
	mov r11301, r10101
	ldr r10601, [r10101]
	movw r144, #0
	cmp r10001, r144
	bge main$L106
main$L105:
	movw r145, #65535
	movt r145, #65535
	mov r0, r145
	bl exit
main$L106:
	cmp r10001, r10601
	bge main$L105
main$L107:
	add r12201, r10001, #1
	mul r12501, r10001, r10001
	movw r133, #4
	mul r12301, r12201, r133
	str r12501, [r11301, r12301]
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
