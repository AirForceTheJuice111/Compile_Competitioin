.balign 4
.global main
.section .text
.arm
main:
main$L110:
	push {r4-r10, fp, lr}
	sub sp, sp, #24
	add fp, sp, #56
	mov r2, #0
	movw r1, #3000
	mov r10, #0
	str r10, [fp, #-40]
	mov r10, #0
	str r10, [fp, #-48]
	add r0, r2, #1
	add r3, r2, #1
	movw r4, #2
	mul r10, r1, r4
	str r10, [fp, #-52]
	movw r1, #2000
	mul r1, r0, r1
	movw r0, #3
	mul r3, r3, r0
	ldr r9, [fp, #-40]
	mov r0, r9
	mov r10, r1
	str r10, [fp, #-56]
	mov r10, r3
	str r10, [fp, #-44]
main$L102:
	ldr r10, [fp, #-52]
	cmp r2, r10
	blt main$L103
main$L104:
	bl putint
	movw r0, #10
	bl putch
	movw r0, #0
	sub sp, fp, #56
	add sp, sp, #24
	pop {r4-r10, fp, lr}
	bx lr
main$L103:
	add r4, r2, #1
	ldr r9, [fp, #-48]
	add r1, r9, #1
	ldr r10, [fp, #-44]
	mul r2, r1, r10
	ldr r9, [fp, #-48]
	mov r1, r9
main$L107:
	ldr r10, [fp, #-56]
	cmp r1, r10
	blt main$L108
main$L109:
	ldr r9, [fp, #-56]
	add r10, r9, #2000
	str r10, [fp, #-56]
	ldr r9, [fp, #-44]
	add r10, r9, #3
	str r10, [fp, #-44]
	mov r2, r4
	ldr r9, [fp, #-56]
	mov r10, r9
	str r10, [fp, #-56]
	ldr r9, [fp, #-44]
	mov r10, r9
	str r10, [fp, #-44]
	b main$L102
main$L108:
	add r3, r1, #1
	add r1, r0, r2
	ldr r10, [fp, #-44]
	add r2, r2, r10
	movw r0, #17
	sdiv r0, r3, r0
	add r0, r1, r0
	mov r1, r3
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
