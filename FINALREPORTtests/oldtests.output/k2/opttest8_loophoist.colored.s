.balign 4
.global main
.section .text
.arm
main:
main$L110:
	push {r4-r10, fp, lr}
	sub sp, sp, #20
	add fp, sp, #52
	mov r10, #0
	str r10, [fp, #-44]
	mov r0, #0
	mov r0, #0
	movw r10, #100
	str r10, [fp, #-40]
	movw r1, #200
	ldr r9, [fp, #-40]
	add r10, r9, r1
	str r10, [fp, #-52]
	ldr r9, [fp, #-52]
	mov r10, r9
	str r10, [fp, #-52]
	ldr r9, [fp, #-44]
	mov r10, r9
	str r10, [fp, #-48]
main$L102:
	movw r1, #10
	ldr r9, [fp, #-48]
	cmp r9, r1
	blt main$L103
main$L104:
	sub sp, fp, #52
	add sp, sp, #20
	pop {r4-r10, fp, lr}
	bx lr
main$L103:
	movw r1, #5
	ldr r9, [fp, #-48]
	cmp r9, r1
	bgt main$L107
main$L108:
main$L109:
	ldr r9, [fp, #-48]
	add r10, r9, #1
	str r10, [fp, #-48]
	ldr r9, [fp, #-48]
	mov r10, r9
	str r10, [fp, #-48]
	b main$L102
main$L107:
	ldr r9, [fp, #-52]
	mov r0, r9
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
