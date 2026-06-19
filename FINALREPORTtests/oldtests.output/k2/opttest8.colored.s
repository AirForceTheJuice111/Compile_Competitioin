.balign 4
.global main
.section .text
.arm
main:
main$L110:
	push {r4-r10, fp, lr}
	sub sp, sp, #8
	add fp, sp, #40
	mov r1, #0
	mov r0, #0
	mov r0, #0
	mov r10, r1
	str r10, [fp, #-40]
main$L102:
	movw r1, #10
	ldr r9, [fp, #-40]
	cmp r9, r1
	blt main$L103
main$L104:
	sub sp, fp, #40
	add sp, sp, #8
	pop {r4-r10, fp, lr}
	bx lr
main$L103:
	movw r1, #5
	ldr r9, [fp, #-40]
	cmp r9, r1
	bgt main$L107
main$L108:
main$L109:
	ldr r9, [fp, #-40]
	add r10, r9, #1
	str r10, [fp, #-40]
	ldr r9, [fp, #-40]
	mov r10, r9
	str r10, [fp, #-40]
	b main$L102
main$L107:
	movw r1, #100
	movw r0, #200
	add r0, r1, r0
	b main$L109

.global malloc
.global getint
.global getch
.global getarray
.global putint
.global putch
.global putarray
.global starttime
.global stoptime
