.balign 4
.global main
.section .text
.arm
main:
main$L100:
	push {r4-r10, fp, lr}
	sub sp, sp, #4
	add fp, sp, #36
	add r3, r4, #4
	mov r2, r6
	add r1, r6, #8
	add r0, r6, #12
	ldr r7, [r4]
	mov r8, r6
	add r5, r6, #8
	add r4, r6, #12
	ldr r3, [r3]
	ldr r2, [r2]
	ldr r1, [r1]
	ldr r0, [r0]
	add r3, r7, r3
	mov r7, r6
	add r6, r6, #4
	add r2, r3, r2
	add r1, r2, r1
	add r0, r1, r0
	bl putint
	ldr r3, [r7]
	ldr r6, [r6]
	ldr r2, [r8]
	ldr r1, [r5]
	ldr r0, [r4]
	add r3, r3, r6
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
