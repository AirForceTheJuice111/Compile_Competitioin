.balign 4
.global main
.section .text
.arm
main:
main$L105:
	push {r4-r10, fp, lr}
	sub sp, sp, #4
	add fp, sp, #36
	movw r1, #2
	movw r0, #3
	add r0, r1, r0
	movw r2, #1
	movw r1, #0
	cmp r2, r1
	bne main$L102
main$L103:
	bl getint
main$L104:
	sub sp, fp, #36
	add sp, sp, #4
	pop {r4-r10, fp, lr}
	bx lr
main$L102:
	mul r0, r0, r0
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
