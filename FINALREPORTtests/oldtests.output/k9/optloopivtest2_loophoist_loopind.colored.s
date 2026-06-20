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
	movw r1, #4
	mul r1, r0, r1
	add r4, r1, #2
	mov r5, r0
main$L102:
	movw r0, #2
	cmp r4, r0
	bgt main$L103
main$L104:
	mov r0, r5
	bl putint
	movw r0, #10
	bl putch
	movw r0, #2
	sub sp, fp, #36
	add sp, sp, #4
	pop {r4-r10, fp, lr}
	bx lr
main$L103:
	mov r0, r4
	bl putint
	sub r5, r5, #2
	sub r4, r4, #8
	movw r0, #32
	bl putch
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
