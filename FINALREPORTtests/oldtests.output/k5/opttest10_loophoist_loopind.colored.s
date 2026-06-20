.balign 4
.global main
.section .text
.arm
main:
main$L110:
	push {r4-r10, fp, lr}
	sub sp, sp, #4
	add fp, sp, #36
	bl getint
	mov r2, r0
	mov r0, #10
	mov r1, #0
	add r1, r1, #1
	movw r3, #3
	mul r3, r1, r3
	mov r1, r4
	mov r4, r3
main$L102:
	movw r3, #0
	cmp r0, r3
	bgt main$L103
main$L104:
	mov r0, r1
	sub sp, fp, #36
	add sp, sp, #4
	pop {r4-r10, fp, lr}
	bx lr
main$L103:
main$L107:
	movw r3, #0
	cmp r2, r3
	bgt main$L108
main$L109:
	sub r0, r0, #1
	add r4, r4, #3
	b main$L102
main$L108:
	sub r2, r2, #1
	mov r1, r4
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
