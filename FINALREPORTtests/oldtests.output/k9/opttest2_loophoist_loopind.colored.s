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
	mov r2, r0
	mov r1, #10
	mov r3, #3
	mov r0, r4
main$L102:
	movw r4, #0
	cmp r1, r4
	bgt main$L103
main$L104:
	sub sp, fp, #36
	add sp, sp, #4
	pop {r4-r10, fp, lr}
	bx lr
main$L103:
main$L107:
	movw r4, #0
	cmp r2, r4
	bgt main$L108
main$L109:
	sub r1, r1, #1
	b main$L102
main$L108:
	sub r2, r2, #1
	mov r0, r3
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
