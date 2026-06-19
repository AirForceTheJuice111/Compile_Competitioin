.balign 4
.global main
.section .text
.arm
main:
main$L110:
	push {r4-r10, fp, lr}
	sub sp, sp, #4
	add fp, sp, #36
	mov r0, #1
	mov r2, #2
	movw r1, #0
	cmp r0, r1
	bne main$L107
main$L108:
	bl putint
main$L109:
main$L111:
	movw r0, #0
	sub sp, fp, #36
	add sp, sp, #4
	pop {r4-r10, fp, lr}
	bx lr
main$L107:
	mov r1, #0
	cmp r0, r2
	blt main$L104
main$L105:
	add r0, r1, r0
	bl putint
	b main$L109
main$L104:
	mov r1, #1
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
