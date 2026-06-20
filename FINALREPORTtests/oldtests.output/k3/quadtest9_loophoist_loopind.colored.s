.balign 4
.global B$f
.section .text
.arm
B$f:
B$f$L100:
	push {r4-r10, fp, lr}
	sub sp, sp, #4
	add fp, sp, #36
	movw r1, #4
	str r1, [r0]
	movw r1, #30
	str r1, [r0, #8]
	movw r1, #100
	str r1, [r0, #12]
	ldr r2, [r0]
	ldr r1, [r0, #8]
	ldr r0, [r0, #12]
	add r1, r2, r1
	add r0, r1, r0
	sub sp, fp, #36
	add sp, sp, #4
	pop {r4-r10, fp, lr}
	bx lr

.balign 4
.global A$f
.section .text
.arm
A$f:
A$f$L100:
	push {r4-r10, fp, lr}
	sub sp, sp, #4
	add fp, sp, #36
	movw r1, #1
	str r1, [r0]
	movw r1, #2
	str r1, [r0, #4]
	movw r0, #0
	sub sp, fp, #36
	add sp, sp, #4
	pop {r4-r10, fp, lr}
	bx lr

.balign 4
.global main
.section .text
.arm
main:
main$L100:
	push {r4-r10, fp, lr}
	sub sp, sp, #52
	add fp, sp, #84
	movw r0, #20
	bl malloc
	ldr r1, =A$f
	str r1, [r0, #16]
	mov r10, r0
	str r10, [fp, #-48]
	movw r0, #20
	bl malloc
	mov r1, r0
	ldr r9, [fp, #-48]
	add r0, r9, #16
	ldr r9, [fp, #-48]
	mov r10, r9
	str r10, [fp, #-48]
	ldr r9, [fp, #-48]
	add r10, r9, #4
	str r10, [fp, #-56]
	ldr r2, =B$f
	str r2, [r1, #16]
	mov r10, r1
	str r10, [fp, #-44]
	ldr r1, [r0]
	ldr r9, [fp, #-48]
	mov r0, r9
	blx r1
	ldr r9, [fp, #-44]
	ldr r1, [r9, #16]
	ldr r9, [fp, #-44]
	mov r0, r9
	blx r1
	ldr r9, [fp, #-44]
	mov r2, r9
	ldr r9, [fp, #-44]
	add r1, r9, #8
	ldr r9, [fp, #-44]
	add r0, r9, #12
	ldr r9, [fp, #-44]
	mov r10, r9
	str r10, [fp, #-40]
	ldr r9, [fp, #-44]
	mov r10, r9
	str r10, [fp, #-72]
	ldr r9, [fp, #-44]
	add r10, r9, #8
	str r10, [fp, #-76]
	ldr r9, [fp, #-44]
	add r10, r9, #12
	str r10, [fp, #-84]
	ldr r9, [fp, #-48]
	ldr r10, [r9]
	str r10, [fp, #-52]
	ldr r9, [fp, #-56]
	ldr r10, [r9]
	str r10, [fp, #-60]
	ldr r10, [r2]
	str r10, [fp, #-64]
	ldr r2, [r1]
	ldr r0, [r0]
	ldr r9, [fp, #-40]
	mov r10, r9
	str r10, [fp, #-68]
	ldr r9, [fp, #-52]
	ldr r10, [fp, #-60]
	add r1, r9, r10
	ldr r10, [fp, #-64]
	add r1, r1, r10
	add r1, r1, r2
	add r0, r1, r0
	bl putint
	movw r0, #10
	bl putch
	ldr r9, [fp, #-68]
	ldr r2, [r9]
	ldr r9, [fp, #-72]
	ldr r1, [r9]
	ldr r9, [fp, #-76]
	ldr r10, [r9]
	str r10, [fp, #-80]
	ldr r9, [fp, #-84]
	ldr r0, [r9]
	add r1, r2, r1
	ldr r10, [fp, #-80]
	add r1, r1, r10
	add r0, r1, r0
	bl putint
	movw r0, #10
	bl putch
	movw r0, #1
	sub sp, fp, #84
	add sp, sp, #52
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
