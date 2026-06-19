.balign 4
.global main
.section .text
.arm
main:
main$L110:
	push {r4-r10, fp, lr}
	sub sp, sp, #20
	add fp, sp, #52
	movw r0, #3000
	mov r2, #0
	mov r10, #0
	str r10, [fp, #-48]
	mov r1, #0
	movw r3, #2
	mul r10, r0, r3
	str r10, [fp, #-52]
	mov r0, r2
main$L102:
	ldr r10, [fp, #-52]
	cmp r1, r10
	blt main$L103
main$L104:
	bl putint
	movw r0, #10
	bl putch
	movw r0, #0
	sub sp, fp, #52
	add sp, sp, #20
	pop {r4-r10, fp, lr}
	bx lr
main$L103:
	add r10, r1, #1
	str r10, [fp, #-44]
	movw r1, #2000
	ldr r9, [fp, #-44]
	mul r4, r9, r1
	movw r1, #3
	ldr r9, [fp, #-44]
	mul r10, r9, r1
	str r10, [fp, #-40]
	ldr r9, [fp, #-48]
	mov r1, r9
	mov r3, r0
main$L107:
	cmp r1, r4
	blt main$L108
main$L109:
	ldr r9, [fp, #-44]
	mov r1, r9
	mov r0, r3
	b main$L102
main$L108:
	add r2, r1, #1
	ldr r10, [fp, #-40]
	mul r1, r2, r10
	movw r0, #17
	sdiv r0, r2, r0
	add r1, r3, r1
	add r3, r1, r0
	mov r1, r2
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
