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
	movw r102, #0
	mov r0, r102
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
	movw r122, #12
	mov r0, r122
	bl malloc
	mov r10301, r0
	ldr r123, =A$f
	str r123, [r10301, #8]
	mov r10001, r10301
	movw r124, #1
	str r124, [r10001]
	mov r11601, r10001
	movw r125, #12
	mov r0, r125
	bl malloc
	mov r10401, r0
	ldr r126, =B$f
	str r126, [r10401, #8]
	mov r10101, r10401
	movw r127, #10
	str r127, [r10101, #4]
	add r11801, r10101, #4
	ldr r11701, [r11601]
	ldr r11901, [r11801]
	add r10701, r11701, r11901
	mov r0, r10701
	bl putint
	movw r128, #10
	mov r0, r128
	bl putch
	movw r129, #0
	mov r0, r129
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
	movw r102, #0
	mov r0, r102
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
