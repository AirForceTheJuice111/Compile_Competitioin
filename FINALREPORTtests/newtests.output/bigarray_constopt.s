.balign 4
.global main
.section .text
.arm
main:
main$L116:
	push {r4-r10, fp, lr}
	sub sp, sp, #4
	add fp, sp, #36
	mov r142, #0
	mov r143, #0
	mov r10502, r142
	mov r10202, r143
main$L102:
	movw r156, #1030
	cmp r10202, r156
	blt main$L103
main$L104:
	movw r154, #10
	mov r0, r154
	bl putch
	mov r0, r10502
	bl putint
	movw r155, #10
	mov r0, r155
	bl putch
	movw r157, #0
	mov r0, r157
	sub sp, fp, #36
	add sp, sp, #4
	pop {r4-r10, fp, lr}
	bx lr
main$L103:
	add r10203, r10202, #1
	mov r144, #0
	movw r147, #1000
	mul r10701, r10203, r147
	movw r148, #1000
	mul r12601, r10203, r148
	add r12401, r10701, #1
	add r10503, r10502, r12601
	movw r149, #4
	mul r11201, r12401, r149
	mov r0, r11201
	bl malloc
	mov r10801, r0
	str r10701, [r10801]
	mov r10001, r10801
	mov r10302, r144
main$L107:
	movw r150, #10
	mul r12701, r10203, r150
	cmp r10302, r12701
	blt main$L108
main$L109:
	sub r11001, r10302, #1
	mov r11701, r10001
	ldr r11101, [r10001]
	movw r158, #0
	cmp r11001, r158
	bge main$L114
main$L113:
	movw r159, #65535
	movt r159, #65535
	mov r0, r159
	bl exit
main$L114:
	cmp r11001, r11101
	bge main$L113
main$L115:
	add r13401, r11001, #1
	movw r152, #4
	mul r13501, r13401, r152
	ldr r11801, [r11701, r13501]
	mov r0, r11801
	bl putint
	movw r153, #32
	mov r0, r153
	bl putch
	mov r10502, r10503
	mov r10202, r10203
	b main$L102
main$L108:
	mov r11501, r10001
	ldr r10901, [r10001]
	movw r160, #0
	cmp r10302, r160
	bge main$L111
main$L110:
	movw r161, #65535
	movt r161, #65535
	mov r0, r161
	bl exit
main$L111:
	cmp r10302, r10901
	bge main$L110
main$L112:
	add r12901, r10302, #1
	mul r13201, r10302, r10203
	add r10303, r10302, #1
	movw r151, #4
	mul r13001, r12901, r151
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
