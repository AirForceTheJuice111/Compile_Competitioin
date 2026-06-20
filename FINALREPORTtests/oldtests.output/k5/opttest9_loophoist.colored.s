.balign 4
.global main
.section .text
.arm
main:
main$L110:
	push {r4-r10, fp, lr}
	sub sp, sp, #16
	add fp, sp, #48
	mov r4, #10
	mov r0, #0
	mov r10, #100
	str r10, [fp, #-44]
	mov r10, #10
	str r10, [fp, #-40]
	bl getint
	mov r1, r0
	ldr r9, [fp, #-40]
	add r0, r9, #1
	ldr r9, [fp, #-40]
	add r0, r9, r0
	movw r2, #3
	mul r10, r2, r0
	str r10, [fp, #-48]
	mov r0, r4
	ldr r9, [fp, #-44]
	mov r2, r9
	mov r3, r1
main$L102:
	movw r1, #0
	cmp r0, r1
	bgt main$L103
main$L104:
	mov r0, r2
	sub sp, fp, #48
	add sp, sp, #16
	pop {r4-r10, fp, lr}
	bx lr
main$L103:
	mov r1, r2
main$L107:
	movw r2, #0
	cmp r3, r2
	bgt main$L108
main$L109:
	sub r0, r0, #1
	mov r2, r1
	b main$L102
main$L108:
	sub r3, r3, #1
	ldr r9, [fp, #-48]
	mov r1, r9
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
