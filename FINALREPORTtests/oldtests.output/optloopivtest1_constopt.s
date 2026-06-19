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
	movw r117, #0
	cmp r10002, r117
	bgt main$L103
main$L104:
	movw r116, #10
	mov r0, r116
	bl putch
	movw r118, #2
	mov r0, r118
	sub sp, fp, #36
	add sp, sp, #4
	pop {r4-r10, fp, lr}
	bx lr
main$L103:
	sub r10003, r10002, #1
	movw r114, #4
	mul r10801, r114, r10003
	add r10301, r10801, #2
	mov r0, r10301
	bl putint
	movw r115, #32
	mov r0, r115
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
