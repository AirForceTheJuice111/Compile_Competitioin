.balign 4
.global main
.section .text
.arm
main:
main$L105:
	push {r4-r10, fp, lr}
	sub sp, sp, #4
	add fp, sp, #36
	mov r112, #10
	mov r10102, r112
main$L102:
	movw r118, #0
	cmp r10102, r118
	bgt main$L103
main$L104:
	mov r0, r10102
	bl putint
	movw r117, #10
	mov r0, r117
	bl putch
	mov r0, r10102
	sub sp, fp, #36
	add sp, sp, #4
	pop {r4-r10, fp, lr}
	bx lr
main$L103:
	sub r10103, r10102, #1
	movw r115, #4
	mul r10701, r115, r10103
	sub r10001, r10701, #2
	mov r0, r10001
	bl putint
	movw r116, #32
	mov r0, r116
	bl putch
	mov r10102, r10103
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
