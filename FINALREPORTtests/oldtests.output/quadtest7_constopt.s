.balign 4
.global main
.section .text
.arm
main:
main$L100:
	push {r4-r10, fp, lr}
	sub sp, sp, #4
	add fp, sp, #36
	movw r124, #12
	mov r0, r124
	bl malloc
	mov r1030101, r0
	ldr r125, =A$f
	str r125, [r1030101, #8]
	mov r1000101, r1030101
	movw r126, #1
	str r126, [r1000101]
	mov r1160101, r1000101
	movw r127, #12
	mov r0, r127
	bl malloc
	mov r1040101, r0
	ldr r128, =B$f
	str r128, [r1040101, #8]
	mov r1010101, r1040101
	movw r129, #10
	str r129, [r1010101, #4]
	add r1180101, r1010101, #4
	ldr r1170101, [r1160101]
	ldr r1190101, [r1180101]
	add r1070101, r1170101, r1190101
	mov r0, r1070101
	bl putint
	movw r130, #10
	mov r0, r130
	bl putch
	movw r131, #0
	mov r0, r131
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
	mov r100, r0
B$f$L101:
	movw r104, #0
	mov r0, r104
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
A$f$L101:
	movw r104, #0
	mov r0, r104
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
