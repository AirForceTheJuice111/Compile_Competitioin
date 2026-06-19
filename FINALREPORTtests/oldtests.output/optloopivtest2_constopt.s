.balign 4
.global main
.section .text
.arm
main:
main$L105:
	push {r4-r10, fp, lr}
	sub sp, sp, #4
	add fp, sp, #36
	bl getint
	mov r10001, r0
	mov r10002, r10001
main$L102:
	movw r119, #0
	cmp r10002, r119
	bgt main$L103
main$L104:
	mov r0, r10002
	bl putint
	movw r118, #10
	mov r0, r118
	bl putch
	movw r120, #2
	mov r0, r120
	sub sp, fp, #36
	add sp, sp, #4
	pop {r4-r10, fp, lr}
	bx lr
main$L103:
	movw r116, #4
	mul r10901, r116, r10002
	sub r10003, r10002, #2
	add r10301, r10901, #2
	mov r0, r10301
	bl putint
	movw r117, #32
	mov r0, r117
	bl putch
	mov r10002, r10003
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
