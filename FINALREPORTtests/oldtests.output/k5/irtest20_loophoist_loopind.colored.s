.balign 4
.global B$f
.section .text
.arm
B$f:
B$f$L100:
	push {r4-r10, fp, lr}
	sub sp, sp, #4
	add fp, sp, #36
	add r2, r0, #8
	add r1, r0, #12
	ldr r3, [r0]
	ldr r2, [r2]
	ldr r0, [r1]
	add r1, r3, r2
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
	sub sp, sp, #20
	add fp, sp, #52
	ldr r4, [r0]
	ldr r3, [r0, #4]
	ldr r9, [fp, #-40]
	ldr r2, [r9]
	ldr r9, [fp, #-40]
	ldr r1, [r9, #8]
	ldr r9, [fp, #-40]
	ldr r0, [r9, #12]
	ldr r9, [fp, #-40]
	mov r10, r9
	str r10, [fp, #-40]
	ldr r9, [fp, #-40]
	mov r10, r9
	str r10, [fp, #-44]
	ldr r9, [fp, #-40]
	add r10, r9, #8
	str r10, [fp, #-48]
	ldr r9, [fp, #-40]
	add r10, r9, #12
	str r10, [fp, #-52]
	add r3, r4, r3
	add r2, r3, r2
	add r1, r2, r1
	add r0, r1, r0
	bl putint
	ldr r9, [fp, #-40]
	ldr r4, [r9]
	ldr r9, [fp, #-40]
	ldr r3, [r9, #4]
	ldr r9, [fp, #-44]
	ldr r2, [r9]
	ldr r9, [fp, #-48]
	ldr r1, [r9]
	ldr r9, [fp, #-52]
	ldr r0, [r9]
	add r3, r4, r3
	add r2, r3, r2
	add r1, r2, r1
	add r0, r1, r0
	bl putint
main$L101:
	movw r0, #0
	sub sp, fp, #52
	add sp, sp, #20
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
