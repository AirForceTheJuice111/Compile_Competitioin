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
	mov r1, #10
	mov r2, #100
main$L102:
	movw r3, #0
	cmp r1, r3
	bgt main$L103
main$L104:
	mov r0, r2
	sub sp, fp, #36
	add sp, sp, #4
	pop {r4-r10, fp, lr}
	bx lr
main$L103:
main$L107:
	movw r3, #0
	cmp r0, r3
	bgt main$L108
main$L109:
	sub r1, r1, #1
	b main$L102
main$L108:
	sub r0, r0, #1
	mov r2, #63
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
