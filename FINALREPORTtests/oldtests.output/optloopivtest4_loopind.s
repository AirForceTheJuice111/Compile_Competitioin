.balign 4
.global main
.section .text
.arm
main:
main$L105:
	push {r4-r10, fp, lr}
	sub sp, sp, #4
	add fp, sp, #36
	mov r10101, #10
	sub r10005, r10101, #1
	movw r112, #4
	mul r10006, r10005, r112
	sub r10003, r10006, #2
	mov r10102, r10101
	mov r10002, r10003
main$L102:
	movw r115, #65530
	movt r115, #65535
	cmp r10002, r115
	bgt main$L103
main$L104:
	mov r0, r10102
	bl putint
	movw r114, #10
	mov r0, r114
	bl putch
	mov r0, r10102
	sub sp, fp, #36
	add sp, sp, #4
	pop {r4-r10, fp, lr}
	bx lr
main$L103:
	sub r10103, r10102, #1
	mov r0, r10002
	bl putint
	sub r10004, r10002, #4
	movw r113, #32
	mov r0, r113
	bl putch
	mov r10102, r10103
	mov r10002, r10004
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
