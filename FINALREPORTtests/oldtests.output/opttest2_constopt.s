.balign 4
.global main
.section .text
.arm
main:
main$L110:
	push {r4-r10, fp, lr}
	sub sp, sp, #4
	add fp, sp, #36
	bl getint
	mov r10001, r0
	mov r106, #10
	mov r10202, r106
	mov r10101, r101
	mov r10002, r10001
main$L102:
	movw r110, #0
	cmp r10202, r110
	bgt main$L103
main$L104:
	mov r0, r10101
	sub sp, fp, #36
	add sp, sp, #4
	pop {r4-r10, fp, lr}
	bx lr
main$L103:
	mov r10102, r10101
	mov r10003, r10002
main$L107:
	movw r111, #0
	cmp r10003, r111
	bgt main$L108
main$L109:
	sub r10203, r10202, #1
	mov r10202, r10203
	mov r10101, r10102
	mov r10002, r10003
	b main$L102
main$L108:
	sub r10004, r10003, #1
	mov r107, #3
	mov r10102, r107
	mov r10003, r10004
	b main$L107

.global malloc
.global getint
.global getch
.global getarray
.global putint
.global putch
.global putarray
.global starttime
.global stoptime
