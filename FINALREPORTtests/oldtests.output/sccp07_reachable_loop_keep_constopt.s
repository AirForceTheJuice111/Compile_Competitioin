.balign 4
.global main
.section .text
.arm
main:
main$L105:
	push {r4-r10, fp, lr}
	sub sp, sp, #4
	add fp, sp, #36
	mov r104, #0
	mov r105, #0
	mov r10102, r104
	mov r10002, r105
main$L102:
	movw r108, #3
	cmp r10002, r108
	blt main$L103
main$L104:
	mov r0, r10102
	sub sp, fp, #36
	add sp, sp, #4
	pop {r4-r10, fp, lr}
	bx lr
main$L103:
	add r10003, r10002, #1
	mov r103, #5
	mov r10102, r103
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
