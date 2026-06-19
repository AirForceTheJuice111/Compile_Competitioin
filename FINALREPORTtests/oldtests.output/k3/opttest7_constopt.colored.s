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
	mov r0, #0
main$L102:
	movw r1, #0
	cmp r2, r1
	bgt main$L103
main$L104:
	sub sp, fp, #36
	add sp, sp, #4
	pop {r4-r10, fp, lr}
	bx lr
main$L103:
	movw r1, #0
	cmp r0, r1
	bne main$L107
main$L108:
main$L109:
	sub r2, r2, #1
	mov r0, #4
	b main$L102
main$L107:
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
