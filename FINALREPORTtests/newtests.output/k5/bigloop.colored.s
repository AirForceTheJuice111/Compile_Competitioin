.balign 4
.global main
.section .text
.arm
main:
main$L110:
	push {r4-r10, fp, lr}
	sub sp, sp, #8
	add fp, sp, #40
	movw r3, #3000
	mov r0, #0
	mov r10, #0
	str r10, [fp, #-40]
	ldr r9, [fp, #-40]
	mov r10, r9
	str r10, [fp, #-40]
main$L102:
	movw r1, #2
	mul r1, r3, r1
	ldr r9, [fp, #-40]
	cmp r9, r1
	blt main$L103
main$L104:
	bl putint
	movw r0, #10
	bl putch
	movw r0, #0
	sub sp, fp, #40
	add sp, sp, #8
	pop {r4-r10, fp, lr}
	bx lr
main$L103:
	ldr r9, [fp, #-40]
	add r10, r9, #1
	str r10, [fp, #-40]
	mov r2, #0
main$L107:
	movw r1, #2000
	ldr r9, [fp, #-40]
	mul r1, r9, r1
	cmp r2, r1
	blt main$L108
main$L109:
	ldr r9, [fp, #-40]
	mov r10, r9
	str r10, [fp, #-40]
	b main$L102
main$L108:
	add r4, r2, #1
	movw r1, #3
	ldr r9, [fp, #-40]
	mul r1, r9, r1
	mul r2, r4, r1
	movw r1, #17
	sdiv r1, r4, r1
	add r0, r0, r2
	add r0, r0, r1
	mov r2, r4
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
