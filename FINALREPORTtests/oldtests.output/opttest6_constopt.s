.balign 4
.global main
.section .text
.arm
main:
main$L115:
	push {r4-r10, fp, lr}
	sub sp, sp, #4
	add fp, sp, #36
	bl getint
	mov r10101, r0
	mov r116, #10
	mov r117, #0
	mov r10302, r116
	mov r10201, r102
	mov r10102, r10101
	mov r10002, r117
main$L102:
	movw r124, #0
	cmp r10302, r124
	bgt main$L103
main$L104:
	mov r0, r10201
	sub sp, fp, #36
	add sp, sp, #4
	pop {r4-r10, fp, lr}
	bx lr
main$L103:
	add r10003, r10002, #1
	mov r10202, r10201
	mov r10103, r10102
main$L107:
	movw r125, #0
	cmp r10103, r125
	bgt main$L108
main$L109:
	sub r10303, r10302, #1
	mov r10302, r10303
	mov r10201, r10202
	mov r10102, r10103
	mov r10002, r10003
	b main$L102
main$L108:
	movw r126, #3
	cmp r10003, r126
	bgt main$L112
main$L113:
	movw r121, #4
	mov r0, r121
	bl malloc
	mov r10501, r0
	ldr r122, =C$m
	str r122, [r10501]
	mov r10601, r10501
	ldr r10901, [r10601]
	movw r123, #3
	mov r0, r10601
	mov r1, r123
	blx r10901
	mov r11001, r0
	mov r10204, r11001
	mov r10205, r10204
main$L114:
	sub r10104, r10103, #1
	mov r10202, r10205
	mov r10103, r10104
	b main$L107
main$L112:
	movw r120, #3
	mul r10203, r120, r10003
	mov r10205, r10203
	b main$L114

.balign 4
.global C$m
.section .text
.arm
C$m:
C$m$L105:
	push {r4-r10, fp, lr}
	sub sp, sp, #4
	add fp, sp, #36
	mov r101, r0
	mov r102, r1
	mov r105, #0
	mov r10201, r102
	mov r10002, r105
C$m$L102:
	movw r108, #0
	cmp r10201, r108
	bne C$m$L103
C$m$L104:
	mov r0, r10002
	sub sp, fp, #36
	add sp, sp, #4
	pop {r4-r10, fp, lr}
	bx lr
C$m$L103:
	sub r10202, r10201, #1
	mov r104, #3
	mov r10201, r10202
	mov r10002, r104
	b C$m$L102

.global malloc
.global getint
.global getch
.global getarray
.global putint
.global putch
.global putarray
.global starttime
.global stoptime
