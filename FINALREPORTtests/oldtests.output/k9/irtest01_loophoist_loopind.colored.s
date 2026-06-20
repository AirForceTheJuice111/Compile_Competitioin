.balign 4
.global main
.section .text
.arm
main:
main$L113:
	push {r4-r10, fp, lr}
	sub sp, sp, #4
	add fp, sp, #36
	mov r4, #0
	bl getint
	mov r5, r0
	movw r0, #0
	cmp r5, r0
	blt main$L105
main$L104:
	movw r0, #47
	cmp r5, r0
	bgt main$L105
main$L106:
main$L107:
main$L110:
	cmp r4, r5
	blt main$L111
main$L112:
	movw r0, #0
	sub sp, fp, #36
	add sp, sp, #4
	pop {r4-r10, fp, lr}
	bx lr
main$L111:
	movw r0, #32
	bl putch
	add r4, r4, #1
	b main$L110
main$L105:
	movw r1, #0
	movw r0, #1
	sub r0, r1, r0
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
