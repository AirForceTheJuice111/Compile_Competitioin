.balign 4
.global main
.section .text
.arm
main:
main$L105:
	push {r4-r10, fp, lr}
	sub sp, sp, #4
	add fp, sp, #36
	bl getint
main$L102:
	movw r1, #0
	cmp r0, r1
	bgt main$L103
main$L104:
	bl putint
	movw r0, #10
	bl putch
	movw r0, #2
	sub sp, fp, #36
	add sp, sp, #4
	pop {r4-r10, fp, lr}
	bx lr
main$L103:
	movw r1, #4
	mul r1, r1, r0
	sub r4, r0, #2
	add r0, r1, #2
	bl putint
	movw r0, #32
	bl putch
	mov r0, r4
	b main$L102

.global malloc
.global getint
.global getch
.global getarray
.global putint
.global putch
.global putarray
.global starttime
.global stoptime
