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
	movw r111, #0
	cmp r10001, r111
	bgt main$L107
main$L108:
	mov r108, #4
	mov r10104, r108
main$L109:
	mov r0, r10104
	sub sp, fp, #36
	add sp, sp, #4
	pop {r4-r10, fp, lr}
	bx lr
main$L107:
	mov r106, #10
	mov r10202, r106
	mov r10101, r101
main$L104:
	movw r112, #0
	cmp r10202, r112
	bgt main$L105
main$L106:
	mov r10104, r10101
	b main$L109
main$L105:
	sub r10203, r10202, #1
	mov r107, #3
	mov r10202, r10203
	mov r10101, r107
	b main$L104

.global malloc
.global getint
.global getch
.global getarray
.global putint
.global putch
.global putarray
.global starttime
.global stoptime
