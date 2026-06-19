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
	mov r10301, #10
	mov r10001, #0
	add r10209, r10001, #1
	movw r107, #3
	mul r10207, r10209, r107
	mov r10302, r10301
	mov r10201, r102
	mov r10102, r10101
	mov r10002, r10001
	mov r10206, r10207
main$L102:
	movw r108, #0
	cmp r10302, r108
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
	movw r109, #0
	cmp r10103, r109
	bgt main$L108
main$L109:
	sub r10303, r10302, #1
	add r10208, r10206, #3
	mov r10302, r10303
	mov r10201, r10202
	mov r10102, r10103
	mov r10002, r10003
	mov r10206, r10208
	b main$L102
main$L108:
	movw r110, #3
	cmp r10003, r110
	bgt main$L112
main$L113:
	mov r10204, r10103
	mov r10205, r10204
main$L114:
	sub r10104, r10103, #1
	mov r10202, r10205
	mov r10103, r10104
	b main$L107
main$L112:
	mov r10205, r10206
	b main$L114

.global malloc
.global getint
.global getch
.global getarray
.global putint
.global putch
.global putarray
.global starttime
.global stoptime
