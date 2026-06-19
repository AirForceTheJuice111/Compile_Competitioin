.balign 4
.global main
.section .text
.arm
main:
main$L105:
	push {r4-r10, fp, lr}
	sub sp, sp, #4
	add fp, sp, #36
	mov r10001, #6
	mov r10002, r10001
main$L102:
	movw r104, #0
	movw r105, #0
	cmp r104, r105
	bne main$L103
main$L104:
	mov r0, r10002
	sub sp, fp, #36
	add sp, sp, #4
	pop {r4-r10, fp, lr}
	bx lr
main$L103:
	bl getint
	mov r10003, r0
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
