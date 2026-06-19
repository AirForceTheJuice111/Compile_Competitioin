.balign 4
.global main
.section .text
.arm
main:
main$L105:
	push {r4-r10, fp, lr}
	sub sp, sp, #4
	add fp, sp, #36
	mov r10201, #0
	bl getint
	mov r10301, r0
	bl getint
	mov r10401, r0
	sub r10207, r10301, r10401
	movw r117, #8
	mul r10209, r10401, r117
	movw r118, #8
	mul r10208, r10207, r118
	add r10205, r10208, #7
	mov r10302, r10301
	mov r10202, r10201
	mov r10204, r10205
main$L102:
	movw r121, #0
	cmp r10302, r121
	bgt main$L103
main$L104:
	movw r120, #10
	mov r0, r120
	bl putch
	mov r0, r10202
	sub sp, fp, #36
	add sp, sp, #4
	pop {r4-r10, fp, lr}
	bx lr
main$L103:
	sub r10303, r10302, r10401
	mov r0, r10204
	bl putint
	sub r10206, r10204, r10209
	movw r119, #32
	mov r0, r119
	bl putch
	mov r10302, r10303
	mov r10202, r10204
	mov r10204, r10206
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
