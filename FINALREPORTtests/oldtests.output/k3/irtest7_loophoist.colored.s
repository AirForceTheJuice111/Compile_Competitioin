.balign 4
.global main
.section .text
.arm
main:
main$L108:
	push {r4-r10, fp, lr}
	sub sp, sp, #12
	add fp, sp, #44
	mov r10, #1
	str r10, [fp, #-40]
	mov r10, #2
	str r10, [fp, #-44]
	bl getint
	mov r2, #0
	movw r1, #1
	ldr r9, [fp, #-40]
	cmp r9, r1
	bgt main$L105
main$L104:
	movw r1, #2
	ldr r9, [fp, #-44]
	cmp r9, r1
	blt main$L105
main$L106:
	add r0, r0, r2
	sub sp, fp, #44
	add sp, sp, #12
	pop {r4-r10, fp, lr}
	bx lr
main$L105:
	mov r2, #1
	b main$L106

.global malloc
.global getint
.global getch
.global getarray
.global putint
.global putch
.global putarray
.global starttime
.global stoptime
