.balign 4
.global main
.section .text
.arm
main:
main$L108:
	push {r4-r10, fp, lr}
	sub sp, sp, #4
	add fp, sp, #36
	mov r5, #1
	mov r4, #2
	bl getint
	mov r1, #0
	movw r2, #1
	cmp r5, r2
	bgt main$L105
main$L104:
	movw r2, #2
	cmp r4, r2
	blt main$L105
main$L106:
	add r0, r0, r1
	sub sp, fp, #36
	add sp, sp, #4
	pop {r4-r10, fp, lr}
	bx lr
main$L105:
	mov r1, #1
	b main$L106

.global malloc
.global getint
.global getch
.global getarray
.global putint
.global putch
.global putarray
.global starttime
.global stoptime
