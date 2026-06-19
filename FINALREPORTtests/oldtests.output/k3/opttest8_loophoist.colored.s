.balign 4
.global main
.section .text
.arm
main:
main$L110:
	push {r4-r10, fp, lr}
	sub sp, sp, #12
	add fp, sp, #44
	mov r10, #0
	str r10, [fp, #-40]
	mov r0, #0
	mov r0, #0
	movw r2, #100
	movw r1, #200
	add r10, r2, r1
	str r10, [fp, #-44]
	ldr r9, [fp, #-44]
	mov r10, r9
	str r10, [fp, #-44]
	ldr r9, [fp, #-40]
	mov r2, r9
main$L102:
	movw r1, #10
	cmp r2, r1
	blt main$L103
main$L104:
	sub sp, fp, #44
	add sp, sp, #12
	pop {r4-r10, fp, lr}
	bx lr
main$L103:
	movw r1, #5
	cmp r2, r1
	bgt main$L107
main$L108:
main$L109:
	add r2, r2, #1
	b main$L102
main$L107:
	ldr r9, [fp, #-44]
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
