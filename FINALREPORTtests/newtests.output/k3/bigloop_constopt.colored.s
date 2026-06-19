.balign 4
.global main
.section .text
.arm
main:
main$L110:
	push {r4-r10, fp, lr}
	sub sp, sp, #12
	add fp, sp, #44
	mov r0, #0
	mov r1, #0
main$L102:
	movw r2, #6000
	cmp r1, r2
	blt main$L103
main$L104:
	bl putint
	movw r0, #10
	bl putch
	movw r0, #0
	sub sp, fp, #44
	add sp, sp, #12
	pop {r4-r10, fp, lr}
	bx lr
main$L103:
	add r10, r1, #1
	str r10, [fp, #-44]
	mov r1, #0
	mov r10, r0
	str r10, [fp, #-40]
main$L107:
	movw r0, #2000
	ldr r9, [fp, #-44]
	mul r0, r9, r0
	cmp r1, r0
	blt main$L108
main$L109:
	ldr r9, [fp, #-44]
	mov r1, r9
	ldr r9, [fp, #-40]
	mov r0, r9
	b main$L102
main$L108:
	add r2, r1, #1
	movw r0, #3
	ldr r9, [fp, #-44]
	mul r0, r9, r0
	mul r1, r2, r0
	movw r0, #17
	sdiv r0, r2, r0
	ldr r9, [fp, #-40]
	add r1, r9, r1
	add r10, r1, r0
	str r10, [fp, #-40]
	mov r1, r2
	ldr r9, [fp, #-40]
	mov r10, r9
	str r10, [fp, #-40]
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
