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

.balign 4
.global main
.section .text
.arm
main:
main$L108:
	push {r4-r10, fp, lr}
	sub sp, sp, #4
	add fp, sp, #36
	movw r135, #8
	mov r0, r135
	bl malloc
	mov r10401, r0
	mov r10201, #3
	movw r136, #20
	mov r0, r136
	bl malloc
	mov r10501, r0
	movw r137, #4
	str r137, [r10501]
	mov r12401, r10401
	add r12501, r10401, #4
	mov r10601, r10401
	sub r13005, r10201, #1
	movw r138, #1
	str r138, [r10501, #4]
	movw r139, #2
	str r139, [r10501, #8]
	movw r140, #3
	str r140, [r10501, #12]
	movw r141, #4
	str r141, [r10501, #16]
	str r10501, [r12401]
	ldr r142, =C$m
	str r142, [r12501]
	ldr r11001, [r10601, #4]
	mov r0, r10601
	blx r11001
	mov r11101, r0
	movw r143, #4
	mul r13006, r13005, r143
	mov r10001, r11101
	add r13003, r13006, #4
	mov r10202, r10201
	mov r13002, r13003
main$L102:
	movw r146, #0
	cmp r13002, r146
	bge main$L103
main$L104:
	movw r145, #10
	mov r0, r145
	bl putch
	movw r147, #2
	mov r0, r147
	sub sp, fp, #36
	add sp, sp, #4
	pop {r4-r10, fp, lr}
	bx lr
main$L103:
	sub r10203, r10202, #1
	mov r11301, r10001
	ldr r10701, [r10001]
	movw r148, #0
	cmp r10203, r148
	bge main$L106
main$L105:
	movw r149, #65535
	movt r149, #65535
	mov r0, r149
	bl exit
main$L106:
	cmp r10203, r10701
	bge main$L105
main$L107:
	ldr r11401, [r11301, r13002]
	sub r13004, r13002, #4
	mov r0, r11401
	bl putint
	movw r144, #32
	mov r0, r144
	bl putch
	mov r10202, r10203
	mov r13002, r13004
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
