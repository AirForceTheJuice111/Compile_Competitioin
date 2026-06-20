.balign 4
.global main
.section .text
.arm
main:
main$L105:
	push {r4-r10, fp, lr}
	sub sp, sp, #4
	add fp, sp, #36
	mov r4, #10
	sub r0, r4, #1
	movw r1, #4
	mul r0, r0, r1
	sub r5, r0, #2
main$L102:
	movw r0, #65530
	movt r0, #65535
	cmp r5, r0
	bgt main$L103
main$L104:
	mov r0, r4
	bl putint
	movw r0, #10
	bl putch
	mov r0, r4
	sub sp, fp, #36
	add sp, sp, #4
	pop {r4-r10, fp, lr}
	bx lr
main$L103:
	sub r4, r4, #1
	mov r0, r5
	bl putint
	sub r5, r5, #4
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
