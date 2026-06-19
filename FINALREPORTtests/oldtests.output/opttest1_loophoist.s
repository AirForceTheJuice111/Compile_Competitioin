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
	mov r10201, #10
	mov r10102, r10001
	mov r10202, r10201
	mov r10101, r101
main$L102:
	movw r106, #0
	cmp r10202, r106
	bgt main$L103
main$L104:
	mov r0, r10101
	sub sp, fp, #36
	add sp, sp, #4
	pop {r4-r10, fp, lr}
	bx lr
main$L103:
	sub r10203, r10202, #1
	mov r10202, r10203
	mov r10101, r10102
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
