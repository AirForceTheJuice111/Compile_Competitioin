.balign 4
.global main
.section .text
.arm
main:
main$L110:
	push {r4-r10, fp, lr}
	sub sp, sp, #4
	add fp, sp, #36
	mov r10001, #1
	mov r10101, #2
	movw r109, #0
	cmp r10001, r109
	bne main$L107
main$L108:
	mov r0, r10001
	bl putint
main$L109:
main$L111:
	movw r110, #0
	mov r0, r110
	sub sp, fp, #36
	add sp, sp, #4
	pop {r4-r10, fp, lr}
	bx lr
main$L107:
	mov r10301, #0
	cmp r10001, r10101
	blt main$L104
	mov r10303, r10301
main$L105:
	add r10401, r10303, r10001
	mov r0, r10401
	bl putint
	b main$L109
main$L104:
	mov r10302, #1
	mov r10303, r10302
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
