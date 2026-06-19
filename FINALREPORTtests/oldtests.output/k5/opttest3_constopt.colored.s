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
	movw r1, #0
	cmp r0, r1
	bgt main$L107
main$L108:
	mov r0, #4
main$L109:
	sub sp, fp, #36
	add sp, sp, #4
	pop {r4-r10, fp, lr}
	bx lr
main$L107:
	mov r0, #10
main$L104:
	movw r1, #0
	cmp r0, r1
	bgt main$L105
main$L106:
	mov r0, r4
	b main$L109
main$L105:
	sub r0, r0, #1
	mov r4, #3
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
