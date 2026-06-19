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
	sub sp, sp, #4
	add fp, sp, #36
	ldr r7, [r0]
	ldr r3, [r0, #4]
	ldr r2, [r8]
	ldr r1, [r8, #8]
	ldr r0, [r8, #12]
	mov r6, r8
	add r5, r8, #8
	add r4, r8, #12
	add r3, r7, r3
	add r2, r3, r2
	add r1, r2, r1
	add r0, r1, r0
	bl putint
	ldr r3, [r8]
	ldr r7, [r8, #4]
	ldr r2, [r6]
	ldr r1, [r5]
	ldr r0, [r4]
	add r3, r3, r7
	add r2, r3, r2
	add r1, r2, r1
	add r0, r1, r0
	bl putint
main$L101:
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
