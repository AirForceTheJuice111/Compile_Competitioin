.balign 4
.global main
.section .text
.arm
main:
main$L110:
	push {r4-r10, fp, lr}
	sub sp, sp, #8
	add fp, sp, #40
	mov r4, #10
	mov r10, #100
	str r10, [fp, #-40]
	bl getint
	mov r1, r0
	ldr r9, [fp, #-40]
	mov r0, r9
main$L102:
	movw r2, #0
	cmp r4, r2
	bgt main$L103
main$L104:
	sub sp, fp, #40
	add sp, sp, #8
	pop {r4-r10, fp, lr}
	bx lr
main$L103:
	mov r3, #10
	add r2, r3, #1
	add r2, r3, r2
main$L107:
	movw r3, #0
	cmp r1, r3
	bgt main$L108
main$L109:
	sub r4, r4, #1
	b main$L102
main$L108:
	movw r0, #3
	mul r0, r0, r2
	sub r1, r1, #1
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
