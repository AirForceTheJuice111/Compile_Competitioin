.balign 4
.global main
.section .text
.arm
main:
main$L100:
	push {r4-r10, fp, lr}
	sub sp, sp, #64
	add fp, sp, #96
	ldr r9, [fp, #-40]
	add r1, r9, #4
	add r10, r0, #8
	str r10, [fp, #-52]
	add r10, r0, #12
	str r10, [fp, #-60]
	ldr r9, [fp, #-40]
	ldr r10, [r9]
	str r10, [fp, #-48]
	mov r10, r0
	str r10, [fp, #-44]
	mov r10, r0
	str r10, [fp, #-76]
	add r10, r0, #8
	str r10, [fp, #-84]
	add r10, r0, #12
	str r10, [fp, #-92]
	ldr r1, [r1]
	ldr r0, [r0]
	ldr r9, [fp, #-52]
	ldr r10, [r9]
	str r10, [fp, #-56]
	ldr r9, [fp, #-60]
	ldr r10, [r9]
	str r10, [fp, #-64]
	ldr r9, [fp, #-48]
	add r1, r9, r1
	ldr r9, [fp, #-44]
	mov r10, r9
	str r10, [fp, #-68]
	ldr r9, [fp, #-44]
	add r10, r9, #4
	str r10, [fp, #-72]
	add r0, r1, r0
	ldr r10, [fp, #-56]
	add r0, r0, r10
	ldr r10, [fp, #-64]
	add r0, r0, r10
	bl putint
	ldr r9, [fp, #-68]
	ldr r1, [r9]
	ldr r9, [fp, #-72]
	ldr r0, [r9]
	ldr r9, [fp, #-76]
	ldr r10, [r9]
	str r10, [fp, #-80]
	ldr r9, [fp, #-84]
	ldr r10, [r9]
	str r10, [fp, #-88]
	ldr r9, [fp, #-92]
	ldr r10, [r9]
	str r10, [fp, #-96]
	add r0, r1, r0
	ldr r10, [fp, #-80]
	add r0, r0, r10
	ldr r10, [fp, #-88]
	add r0, r0, r10
	ldr r10, [fp, #-96]
	add r0, r0, r10
	bl putint
main$L101:
	movw r0, #0
	sub sp, fp, #96
	add sp, sp, #64
	pop {r4-r10, fp, lr}
	bx lr

.balign 4
.global B$f
.section .text
.arm
B$f:
B$f$L100:
	push {r4-r10, fp, lr}
	sub sp, sp, #8
	add fp, sp, #40
	ldr r10, [r0]
	str r10, [fp, #-40]
	ldr r1, [r0, #8]
	ldr r0, [r0, #12]
	ldr r9, [fp, #-40]
	add r1, r9, r1
	add r0, r1, r0
	sub sp, fp, #40
	add sp, sp, #8
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
