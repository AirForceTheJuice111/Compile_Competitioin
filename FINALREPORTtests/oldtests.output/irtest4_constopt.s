.balign 4
.global main
.section .text
.arm
main:
main$L108:
	push {r4-r10, fp, lr}
	sub sp, sp, #4
	add fp, sp, #36
	mov r106, #1
	mov r10002, r106
main$L105:
	movw r110, #0
	cmp r10002, r110
	bgt main$L106
main$L104:
	movw r111, #0
	cmp r10002, r111
	beq main$L106
main$L107:
	mov r0, r10002
	sub sp, fp, #36
	add sp, sp, #4
	pop {r4-r10, fp, lr}
	bx lr
main$L106:
	mov r0, r10002
	bl putint
	sub r10003, r10002, #1
	movw r109, #10
	mov r0, r109
	bl putch
	mov r10002, r10003
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
