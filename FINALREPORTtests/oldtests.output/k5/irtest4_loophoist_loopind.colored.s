.balign 4
.global main
.section .text
.arm
main:
main$L108:
	push {r4-r10, fp, lr}
	sub sp, sp, #4
	add fp, sp, #36
	mov r4, #1
main$L105:
	movw r0, #0
	cmp r4, r0
	bgt main$L106
main$L104:
	movw r0, #0
	cmp r4, r0
	beq main$L106
main$L107:
	mov r0, r4
	sub sp, fp, #36
	add sp, sp, #4
	pop {r4-r10, fp, lr}
	bx lr
main$L106:
	mov r0, r4
	bl putint
	sub r4, r4, #1
	movw r0, #10
	bl putch
	b main$L105

.global malloc
.global getint
.global getch
.global getarray
.global putint
.global putch
.global putarray
.global starttime
.global stoptime
