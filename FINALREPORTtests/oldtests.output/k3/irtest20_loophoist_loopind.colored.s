.balign 4
.global B$f
.section .text
.arm
B$f:
B$f$L100:
	push {r4-r10, fp, lr}
	sub sp, sp, #8
	add fp, sp, #40
	add r2, r0, #8
	add r1, r0, #12
	ldr r10, [r0]
	str r10, [fp, #-40]
	ldr r0, [r2]
	ldr r1, [r1]
	ldr r9, [fp, #-40]
	add r0, r9, r0
	add r0, r0, r1
	sub sp, fp, #40
	add sp, sp, #8
	pop {r4-r10, fp, lr}
	bx lr

.balign 4
.global main
.section .text
.arm
main:
main$L100:
	push {r4-r10, fp, lr}
	sub sp, sp, #40
	add fp, sp, #72
	ldr r10, [r0]
	str r10, [fp, #-48]
	ldr r1, [r0, #4]
	ldr r9, [fp, #-40]
	ldr r2, [r9]
	ldr r9, [fp, #-40]
	ldr r10, [r9, #8]
	str r10, [fp, #-52]
	ldr r9, [fp, #-40]
	ldr r0, [r9, #12]
	ldr r9, [fp, #-40]
	mov r10, r9
	str r10, [fp, #-44]
	ldr r9, [fp, #-40]
	mov r10, r9
	str r10, [fp, #-56]
	ldr r9, [fp, #-40]
	add r10, r9, #8
	str r10, [fp, #-64]
	ldr r9, [fp, #-40]
	add r10, r9, #12
	str r10, [fp, #-72]
	ldr r9, [fp, #-48]
	add r1, r9, r1
	add r1, r1, r2
	ldr r10, [fp, #-52]
	add r1, r1, r10
	add r0, r1, r0
	bl putint
	ldr r9, [fp, #-44]
	ldr r2, [r9]
	ldr r9, [fp, #-44]
	ldr r1, [r9, #4]
	ldr r9, [fp, #-56]
	ldr r10, [r9]
	str r10, [fp, #-60]
	ldr r9, [fp, #-64]
	ldr r10, [r9]
	str r10, [fp, #-68]
	ldr r9, [fp, #-72]
	ldr r0, [r9]
	add r1, r2, r1
	ldr r10, [fp, #-60]
	add r1, r1, r10
	ldr r10, [fp, #-68]
	add r1, r1, r10
	add r0, r1, r0
	bl putint
main$L101:
	movw r0, #0
	sub sp, fp, #72
	add sp, sp, #40
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
