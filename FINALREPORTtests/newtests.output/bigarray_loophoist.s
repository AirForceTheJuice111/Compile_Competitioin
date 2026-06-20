.balign 4
.global main
.section .text
.arm
main:
main$L116:
	push {r4-r10, fp, lr}
	sub sp, sp, #4
	add fp, sp, #36
	movw r10401, #1030
	mov r10101, #1000
	mov r10201, #0
	mov r10501, #0
	mov r10301, #0
	mov r10502, r10501
	mov r10202, r10201
main$L102:
	cmp r10202, r10401
	blt main$L103
main$L104:
	movw r147, #10
	mov r0, r147
	bl putch
	mov r0, r10502
	bl putint
	movw r148, #10
	mov r0, r148
	bl putch
	movw r149, #0
	mov r0, r149
	sub sp, fp, #36
	add sp, sp, #4
	pop {r4-r10, fp, lr}
	bx lr
main$L103:
	add r10203, r10202, #1
	mul r10701, r10203, r10101
	mul r12601, r10203, r10101
	movw r142, #10
	mul r12701, r10203, r142
	add r12401, r10701, #1
	add r10503, r10502, r12601
	movw r143, #4
	mul r11201, r12401, r143
	mov r0, r11201
	bl malloc
	mov r10801, r0
	str r10701, [r10801]
	mov r10001, r10801
	mov r11501, r10001
	mov r10302, r10301
main$L107:
	cmp r10302, r12701
	blt main$L108
main$L109:
	sub r11001, r10302, #1
	mov r11701, r10001
	ldr r11101, [r10001]
	movw r150, #0
	cmp r11001, r150
	bge main$L114
main$L113:
	movw r151, #65535
	movt r151, #65535
	mov r0, r151
	bl exit
main$L114:
	cmp r11001, r11101
	bge main$L113
main$L115:
	add r13401, r11001, #1
	movw r145, #4
	mul r13501, r13401, r145
	ldr r11801, [r11701, r13501]
	mov r0, r11801
	bl putint
	movw r146, #32
	mov r0, r146
	bl putch
	mov r10502, r10503
	mov r10202, r10203
	b main$L102
main$L108:
	ldr r10901, [r10001]
	movw r152, #0
	cmp r10302, r152
	bge main$L111
main$L110:
	movw r153, #65535
	movt r153, #65535
	mov r0, r153
	bl exit
main$L111:
	cmp r10302, r10901
	bge main$L110
main$L112:
	add r12901, r10302, #1
	mul r13201, r10302, r10203
	add r10303, r10302, #1
	movw r144, #4
	mul r13001, r12901, r144
	str r13201, [r11501, r13001]
	mov r10302, r10303
	b main$L107

.global malloc
.global getint
.global getch
.global getarray
.global putint
.global putch
.global putarray
.global starttime
.global stoptime
