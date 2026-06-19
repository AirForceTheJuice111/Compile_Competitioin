.balign 4
.global main
.section .text
.arm
main:
main$L105:
	push {r4-r10, fp, lr}
	sub sp, sp, #4
	add fp, sp, #36
	mov r10001, #10
	mov r10002, r10001
main$L102:
	movw r107, #0
	cmp r10002, r107
	bne main$L103
main$L104:
	mov r0, r10002
	sub sp, fp, #36
	add sp, sp, #4
	pop {r4-r10, fp, lr}
	bx lr
main$L103:
	mov r0, r10002
	bl putint
	sub r10003, r10002, #1
	movw r106, #10
	mov r0, r106
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
