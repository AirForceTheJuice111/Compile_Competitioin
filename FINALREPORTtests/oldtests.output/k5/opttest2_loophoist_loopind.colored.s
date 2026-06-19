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
	mov r2, r0
	mov r0, #10
	mov r4, #3
	ldr r9, [fp, #-40]
	mov r1, r9
main$L102:
	movw r3, #0
	cmp r0, r3
	bgt main$L103
main$L104:
	mov r0, r1
	sub sp, fp, #40
	add sp, sp, #8
	pop {r4-r10, fp, lr}
	bx lr
main$L103:
main$L107:
	movw r3, #0
	cmp r2, r3
	bgt main$L108
main$L109:
	sub r0, r0, #1
	b main$L102
main$L108:
	sub r2, r2, #1
	mov r1, r4
	b main$L107

.global malloc
.global getint
.global getch
.global getarray
.global putint
.global putch
.global putarray
.global starttime
.global stoptime
