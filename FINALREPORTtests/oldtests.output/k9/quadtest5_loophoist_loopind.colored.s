.balign 4
.global main
.section .text
.arm
main:
main$L110:
	push {r4-r10, fp, lr}
	sub sp, sp, #4
	add fp, sp, #36
	mov r1, #1
	mov r0, #2
main$L102:
	movw r2, #0
	cmp r1, r2
	bne main$L103
main$L104:
	movw r0, #100
	bl putint
	movw r0, #10
	bl putch
	movw r0, #1
	sub sp, fp, #36
	add sp, sp, #4
	pop {r4-r10, fp, lr}
	bx lr
main$L103:
	movw r2, #0
	cmp r0, r2
	bne main$L107
main$L108:
	b main$L102
main$L107:
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
