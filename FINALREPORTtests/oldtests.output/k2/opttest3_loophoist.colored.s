.balign 4
.global main
.section .text
.arm
main:
main$L110:
	push {r4-r10, fp, lr}
	sub sp, sp, #20
	add fp, sp, #52
	bl getint
	mov r1, r0
	mov r0, #10
	movw r10, #0
	str r10, [fp, #-44]
	ldr r10, [fp, #-44]
	cmp r1, r10
	bgt main$L107
main$L108:
	mov r0, #4
main$L109:
	sub sp, fp, #52
	add sp, sp, #20
	pop {r4-r10, fp, lr}
	bx lr
main$L107:
	mov r10, #3
	str r10, [fp, #-52]
	ldr r9, [fp, #-40]
	mov r1, r9
main$L104:
	movw r10, #0
	str r10, [fp, #-48]
	ldr r10, [fp, #-48]
	cmp r0, r10
	bgt main$L105
main$L106:
	mov r0, r1
	b main$L109
main$L105:
	sub r0, r0, #1
	ldr r9, [fp, #-52]
	mov r1, r9
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
