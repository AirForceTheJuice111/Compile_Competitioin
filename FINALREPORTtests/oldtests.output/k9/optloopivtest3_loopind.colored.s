.balign 4
.global main
.section .text
.arm
main:
main$L105:
	push {r4-r10, fp, lr}
	sub sp, sp, #4
	add fp, sp, #36
	mov r6, #0
	bl getint
	mov r5, r0
	bl getint
	mov r4, r0
	sub r0, r5, r4
	movw r1, #8
	mul r8, r4, r1
	movw r1, #8
	mul r0, r0, r1
	add r1, r0, #7
	mov r0, r5
	mov r5, r6
	mov r7, r1
main$L102:
	movw r1, #0
	cmp r0, r1
	bgt main$L103
main$L104:
	movw r0, #10
	bl putch
	mov r0, r5
	sub sp, fp, #36
	add sp, sp, #4
	pop {r4-r10, fp, lr}
	bx lr
main$L103:
	sub r5, r0, r4
	mov r0, r7
	bl putint
	sub r6, r7, r8
	movw r0, #32
	bl putch
	mov r0, r5
	mov r5, r7
	mov r7, r6
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
