.balign 4
.global main
.section .text
.arm
main:
main$L116:
	push {r4-r10, fp, lr}
	sub sp, sp, #4
	add fp, sp, #36
	mov r10101, #1000
	mov r10201, #0
	mov r10501, #0
	mov r10301, #0
	movw r10401, #1030
	add r11405, r10201, #1
	add r13205, r10201, #1
	add r13305, r10201, #1
	add r13808, r10201, #1
	mul r13806, r13808, r10301
	mul r11403, r11405, r10101
	mul r13203, r13205, r10101
	movw r148, #10
	mul r13303, r13305, r148
	mov r10502, r10501
	mov r10202, r10201
	mov r11402, r11403
	mov r13202, r13203
	mov r13302, r13303
	mov r13805, r13806
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
	movw r149, #4
	mov r0, r149
	bl malloc
	mov r10701, r0
	add r10503, r10502, r13202
	movw r150, #4
	mul r13605, r10301, r150
	ldr r151, =C$a
	str r151, [r10701]
	mov r10801, r10701
	add r13603, r13605, #4
	ldr r11301, [r10801]
	mov r11501, r10801
	mov r11601, r11301
	mov r0, r11501
	mov r1, r11402
	blx r11601
	mov r11701, r0
	mov r10001, r11701
	mov r11901, r10001
	mov r10302, r10301
	mov r13602, r13603
	mov r13802, r13805
main$L107:
	cmp r10302, r13302
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
	add r11404, r11402, r10101
	add r13204, r13202, r10101
	add r13304, r13302, #10
	add r13807, r13805, r10301
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
	mov r11402, r11404
	mov r13202, r13204
	mov r13302, r13304
	mov r13805, r13807
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
	str r13802, [r11901, r13602]
	add r10303, r10302, #1
	add r13604, r13602, #4
	add r13804, r13802, r10203
	mov r10302, r10303
	mov r13602, r13604
	mov r13802, r13804
	b main$L107

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

.global malloc
.global getint
.global getch
.global getarray
.global putint
.global putch
.global putarray
.global starttime
.global stoptime
