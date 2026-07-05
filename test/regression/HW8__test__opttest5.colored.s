.balign 4
.global main
.section .text
.arm
main:
main$L108:
	push {r4-r10, fp, lr}
	sub sp, sp, #4
	add fp, sp, #36
	bl getint
	mov r1, r0
	mov r2, #0
	movw r0, #0
	cmp r1, r0
	bne main$L102
	mov r1, r2
	mov r2, r4
main$L103:
	movw r0, #0
	cmp r1, r0
	bne main$L106
main$L107:
	movw r0, #65435
	movt r0, #65535
	mov r0, r0
	sub sp, fp, #36
	add sp, sp, #4
	pop {r4-r10, fp, lr}
	bx lr
main$L106:
	mov r0, r2
	sub sp, fp, #36
	add sp, sp, #4
	pop {r4-r10, fp, lr}
	bx lr
main$L102:
	mov r1, #1
	mov r0, #9
	mov r2, r0
	b main$L103

.global malloc
.global getint
.global getch
.global getarray
.global putint
.global putch
.global putarray
.global starttime
.global stoptime
