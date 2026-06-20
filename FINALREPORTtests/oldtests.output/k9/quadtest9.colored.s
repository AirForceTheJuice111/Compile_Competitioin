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
.global main
.section .text
.arm
main:
main$L100:
	push {r4-r10, fp, lr}
	sub sp, sp, #8
	add fp, sp, #40
	movw r0, #20
	bl malloc
	ldr r1, =A$f
	str r1, [r0, #16]
	mov r6, r0
	movw r0, #20
	bl malloc
	add r1, r6, #16
	add r5, r6, #4
	ldr r2, =B$f
	str r2, [r0, #16]
	mov r10, r0
	str r10, [fp, #-40]
	ldr r1, [r1]
	mov r0, r6
	blx r1
	ldr r9, [fp, #-40]
	ldr r1, [r9, #16]
	ldr r9, [fp, #-40]
	mov r0, r9
	blx r1
	ldr r9, [fp, #-40]
	mov r2, r9
	ldr r9, [fp, #-40]
	add r1, r9, #8
	ldr r9, [fp, #-40]
	add r0, r9, #12
	ldr r9, [fp, #-40]
	mov r7, r9
	ldr r9, [fp, #-40]
	mov r10, r9
	str r10, [fp, #-40]
	ldr r9, [fp, #-40]
	add r8, r9, #8
	ldr r9, [fp, #-40]
	add r4, r9, #12
	ldr r3, [r6]
	ldr r6, [r5]
	ldr r2, [r2]
	ldr r1, [r1]
	ldr r0, [r0]
	mov r5, r7
	add r3, r3, r6
	add r2, r3, r2
	add r1, r2, r1
	add r0, r1, r0
	bl putint
	movw r0, #10
	bl putch
	ldr r3, [r5]
	ldr r9, [fp, #-40]
	ldr r2, [r9]
	ldr r1, [r8]
	ldr r0, [r4]
	add r2, r3, r2
	add r1, r2, r1
	add r0, r1, r0
	bl putint
	movw r0, #10
	bl putch
	movw r0, #1
	sub sp, fp, #40
	add sp, sp, #8
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

.global malloc
.global getint
.global getch
.global getarray
.global putint
.global putch
.global putarray
.global starttime
.global stoptime
