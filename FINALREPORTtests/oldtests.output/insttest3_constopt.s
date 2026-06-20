.balign 4
.global main
.section .text
.arm
main:
main$L108:
	push {r4-r10, fp, lr}
	sub sp, sp, #4
	add fp, sp, #36
	movw r138, #8
	mov r0, r138
	bl malloc
	mov r10401, r0
	mov r135, #3
	movw r139, #20
	mov r0, r139
	bl malloc
	mov r10501, r0
	movw r140, #4
	str r140, [r10501]
	mov r12401, r10401
	add r12501, r10401, #4
	mov r10601, r10401
	movw r141, #1
	str r141, [r10501, #4]
	movw r142, #2
	str r142, [r10501, #8]
	movw r143, #3
	str r143, [r10501, #12]
	movw r144, #4
	str r144, [r10501, #16]
	str r10501, [r12401]
	ldr r145, =C$m
	str r145, [r12501]
	ldr r11001, [r10601, #4]
	mov r0, r10601
	blx r11001
	mov r11101, r0
	mov r10001, r11101
	mov r10202, r135
main$L102:
	movw r149, #0
	cmp r10202, r149
	bge main$L103
main$L104:
	movw r148, #10
	mov r0, r148
	bl putch
	movw r150, #2
	mov r0, r150
	sub sp, fp, #36
	add sp, sp, #4
	pop {r4-r10, fp, lr}
	bx lr
main$L103:
	sub r10203, r10202, #1
	mov r11301, r10001
	ldr r10701, [r10001]
	movw r151, #0
	cmp r10203, r151
	bge main$L106
main$L105:
	movw r152, #65535
	movt r152, #65535
	mov r0, r152
	bl exit
main$L106:
	cmp r10203, r10701
	bge main$L105
main$L107:
	add r12901, r10203, #1
	movw r146, #4
	mul r13001, r12901, r146
	ldr r11401, [r11301, r13001]
	mov r0, r11401
	bl putint
	movw r147, #32
	mov r0, r147
	bl putch
	mov r10202, r10203
	b main$L102

.balign 4
.global C$m
.section .text
.arm
C$m:
C$m$L100:
	push {r4-r10, fp, lr}
	sub sp, sp, #4
	add fp, sp, #36
	mov r100, r0
	ldr r10301, [r100]
	mov r0, r10301
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
