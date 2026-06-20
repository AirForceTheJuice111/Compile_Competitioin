.balign 4
.global main
.section .text
.arm
main:
main$L110:
	push {r4-r10, fp, lr}
	sub sp, sp, #4
	add fp, sp, #36
	mov r4, #10
	mov r0, #0
	mov r5, #100
	mov r6, #10
	bl getint
	add r1, r6, #1
	add r1, r6, r1
	movw r2, #3
	mul r1, r2, r1
main$L102:
	movw r2, #0
	cmp r4, r2
	bgt main$L103
main$L104:
	mov r0, r5
	sub sp, fp, #36
	add sp, sp, #4
	pop {r4-r10, fp, lr}
	bx lr
main$L103:
main$L107:
	movw r2, #0
	cmp r0, r2
	bgt main$L108
main$L109:
	sub r4, r4, #1
	b main$L102
main$L108:
	sub r0, r0, #1
	mov r5, r1
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
