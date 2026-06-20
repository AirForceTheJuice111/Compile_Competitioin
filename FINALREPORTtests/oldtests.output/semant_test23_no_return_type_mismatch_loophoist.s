.balign 4
.global main
.section .text
.arm
main:
main$L100:
	push {r4-r10, fp, lr}
	sub sp, sp, #4
	add fp, sp, #36
	movw r111, #4
	mov r0, r111
	bl malloc
	mov r1030101, r0
	ldr r112, =A$f
	str r112, [r1030101]
	mov r1000101, r1030101
	ldr r1050101, [r1000101]
	mov r0, r1000101
	blx r1050101
	mov r1010101, r0
	movw r113, #0
	mov r0, r113
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
	mov r100, r0
	mov r1020101, #1
	add r1060101, r1020101, #1
	movw r108, #4
	mul r1040101, r1060101, r108
	mov r0, r1040101
	bl malloc
	mov r1030101, r0
	str r1020101, [r1030101]
	mov r0, r1030101
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
