.balign 4
.global main
.section .text
.arm
main:
main$L113:
	push {r4-r10, fp, lr}
	sub sp, sp, #4
	add fp, sp, #36
	mov r10001, #0
	bl getint
	mov r10101, r0
	movw r111, #0
	cmp r10101, r111
	blt main$L105
main$L104:
	movw r112, #47
	cmp r10101, r112
	bgt main$L105
main$L106:
main$L107:
	mov r10002, r10001
main$L110:
	cmp r10002, r10101
	blt main$L111
main$L112:
	movw r113, #0
	mov r0, r113
	sub sp, fp, #36
	add sp, sp, #4
	pop {r4-r10, fp, lr}
	bx lr
main$L111:
	movw r110, #32
	mov r0, r110
	bl putch
	add r10003, r10002, #1
	mov r10002, r10003
	b main$L110
main$L105:
	movw r108, #0
	movw r109, #1
	sub r10601, r108, r109
	mov r0, r10601
	sub sp, fp, #36
	add sp, sp, #4
	pop {r4-r10, fp, lr}
	bx lr

.global malloc
.global getint
.global getch
.global getarray
.global putint
.global putch
.global putarray
.global starttime
.global stoptime
