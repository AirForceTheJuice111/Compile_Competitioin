.balign 4
.global main
.section .text
.arm
main:
main$L110:
	push {r4-r10, fp, lr}
	sub sp, sp, #8
	add fp, sp, #40
	bl getint
	mov r1, r0
	mov r0, #10
	movw r2, #0
	cmp r1, r2
	bgt main$L107
main$L108:
	mov r0, #4
main$L109:
	sub sp, fp, #40
	add sp, sp, #8
	pop {r4-r10, fp, lr}
	bx lr
main$L107:
	ldr r9, [fp, #-40]
	mov r1, r9
main$L104:
	movw r2, #0
	cmp r0, r2
	bgt main$L105
main$L106:
	mov r0, r1
	b main$L109
main$L105:
	mov r1, #3
	sub r0, r0, #1
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
