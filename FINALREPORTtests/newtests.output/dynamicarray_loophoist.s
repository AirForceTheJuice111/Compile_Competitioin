.balign 4
.global C$a
.section .text
.arm
C$a:
C$a$L100:
	push {r4-r10, fp, lr}
	sub sp, sp, #4
	add fp, sp, #36
	mov r100, r0
	mov r101, r1
	mov r10301, r101
	add r10701, r10301, #1
	movw r109, #4
	mul r10501, r10701, r109
	mov r0, r10501
	bl malloc
	mov r10401, r0
	str r10301, [r10401]
	mov r0, r10401
	sub sp, fp, #36
	add sp, sp, #4
	pop {r4-r10, fp, lr}
	bx lr

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
	movw r154, #10
	mov r0, r154
	bl putch
	mov r0, r10502
	bl putint
	movw r155, #10
	mov r0, r155
	bl putch
	movw r156, #0
	mov r0, r156
	sub sp, fp, #36
	add sp, sp, #4
	pop {r4-r10, fp, lr}
	bx lr
main$L103:
	add r10203, r10202, #1
	movw r148, #4
	mov r0, r148
	bl malloc
	mov r10701, r0
	mul r11401, r10203, r10101
	mul r13201, r10203, r10101
	movw r149, #10
	mul r13301, r10203, r149
	ldr r150, =C$a
	str r150, [r10701]
	mov r10801, r10701
	add r10503, r10502, r13201
	ldr r11301, [r10801]
	mov r11501, r10801
	mov r11601, r11301
	mov r0, r11501
	mov r1, r11401
	blx r11601
	mov r11701, r0
	mov r10001, r11701
	mov r11901, r10001
	mov r10302, r10301
main$L107:
	cmp r10302, r13301
	blt main$L108
main$L109:
	sub r11001, r10302, #1
	mov r12101, r10001
	ldr r11101, [r10001]
	movw r157, #0
	cmp r11001, r157
	bge main$L114
main$L113:
	movw r158, #65535
	movt r158, #65535
	mov r0, r158
	bl exit
main$L114:
	cmp r11001, r11101
	bge main$L113
main$L115:
	add r14001, r11001, #1
	movw r152, #4
	mul r14101, r14001, r152
	ldr r12201, [r12101, r14101]
	mov r0, r12201
	bl putint
	movw r153, #32
	mov r0, r153
	bl putch
	mov r10502, r10503
	mov r10202, r10203
	b main$L102
main$L108:
	ldr r10901, [r10001]
	movw r159, #0
	cmp r10302, r159
	bge main$L111
main$L110:
	movw r160, #65535
	movt r160, #65535
	mov r0, r160
	bl exit
main$L111:
	cmp r10302, r10901
	bge main$L110
main$L112:
	add r13501, r10302, #1
	mul r13801, r10302, r10203
	add r10303, r10302, #1
	movw r151, #4
	mul r13601, r13501, r151
	str r13801, [r11901, r13601]
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
