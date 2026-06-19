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
	mov r5, r0
	mov r6, #0
	bl getint
	mov r4, r0
main$L102:
	movw r0, #0
	cmp r5, r0
	bgt main$L103
main$L104:
	movw r0, #10
	bl putch
	mov r0, r6
	sub sp, fp, #36
	add sp, sp, #4
	pop {r4-r10, fp, lr}
	bx lr
main$L103:
	sub r5, r5, r4
	movw r0, #8
	mul r0, r0, r5
	add r6, r0, #7
	mov r0, r6
	bl putint
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
