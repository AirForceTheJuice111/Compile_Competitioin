.balign 4
.global main
.section .text
.arm
main:
main$L108:
	push {r4-r10, fp, lr}
	sub sp, sp, #4
	add fp, sp, #36
	mov r0, #0
	movw r2, #0
	movw r1, #0
	cmp r2, r1
	bne main$L100
main$L106:
	mov r0, #2
main$L107:
	sub sp, fp, #36
	add sp, sp, #4
	pop {r4-r10, fp, lr}
	bx lr
main$L100:
	movw r1, #10
	sdiv r0, r1, r0
	movw r1, #0
	cmp r0, r1
	beq main$L106
main$L105:
	mov r0, #1
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
