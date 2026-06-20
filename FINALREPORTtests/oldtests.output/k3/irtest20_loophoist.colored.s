.balign 4
.global main
.section .text
.arm
main:
main$L100:
	push {r4-r10, fp, lr}
	sub sp, sp, #52
	add fp, sp, #84
	add r1, r2, #4
	add r10, r0, #8
	str r10, [fp, #-48]
	add r10, r0, #12
	str r10, [fp, #-56]
	ldr r10, [r2]
	str r10, [fp, #-44]
	mov r10, r0
	str r10, [fp, #-40]
	mov r10, r0
	str r10, [fp, #-68]
	add r10, r0, #8
	str r10, [fp, #-76]
	add r10, r0, #12
	str r10, [fp, #-84]
	ldr r2, [r1]
	ldr r1, [r0]
	ldr r9, [fp, #-48]
	ldr r10, [r9]
	str r10, [fp, #-52]
	ldr r9, [fp, #-56]
	ldr r0, [r9]
	ldr r9, [fp, #-44]
	add r2, r9, r2
	ldr r9, [fp, #-40]
	mov r10, r9
	str r10, [fp, #-60]
	ldr r9, [fp, #-40]
	add r10, r9, #4
	str r10, [fp, #-64]
	add r1, r2, r1
	ldr r10, [fp, #-52]
	add r1, r1, r10
	add r0, r1, r0
	bl putint
	ldr r9, [fp, #-60]
	ldr r2, [r9]
	ldr r9, [fp, #-64]
	ldr r1, [r9]
	ldr r9, [fp, #-68]
	ldr r10, [r9]
	str r10, [fp, #-72]
	ldr r9, [fp, #-76]
	ldr r10, [r9]
	str r10, [fp, #-80]
	ldr r9, [fp, #-84]
	ldr r0, [r9]
	add r1, r2, r1
	ldr r10, [fp, #-72]
	add r1, r1, r10
	ldr r10, [fp, #-80]
	add r1, r1, r10
	add r0, r1, r0
	bl putint
main$L101:
	movw r0, #0
	sub sp, fp, #84
	add sp, sp, #52
	pop {r4-r10, fp, lr}
	bx lr

.balign 4
.global B$f
.section .text
.arm
B$f:
B$f$L100:
	push {r4-r10, fp, lr}
	sub sp, sp, #4
	add fp, sp, #36
	ldr r2, [r0]
	ldr r1, [r0, #8]
	ldr r0, [r0, #12]
	add r1, r2, r1
	add r0, r1, r0
	sub sp, fp, #36
	add sp, sp, #4
	pop {r4-r10, fp, lr}
	bx lr

.global malloc
.global getint
.global getch
.global getarray
.global putint
.global putch
.global putarray
.global starttime
.global stoptime
