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
	movw r102, #200
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
	movw r125, #16
	mov r0, r125
	bl malloc
	mov r10301, r0
	ldr r126, =A$f
	str r126, [r10301, #8]
	mov r10001, r10301
	movw r127, #4
	str r127, [r10001]
	movw r128, #16
	mov r0, r128
	bl malloc
	mov r10401, r0
	ldr r129, =B$f
	str r129, [r10401, #8]
	ldr r130, =B$g
	str r130, [r10401, #12]
	mov r10101, r10401
	movw r131, #2
	str r131, [r10101, #4]
	mov r10002, r10101
	add r12101, r10101, #4
	ldr r10701, [r10002, #8]
	mov r0, r10002
	blx r10701
	mov r10801, r0
	ldr r12201, [r12101]
	add r10901, r10801, r12201
	mov r0, r10901
	bl putint
	movw r132, #10
	mov r0, r132
	bl putch
	movw r133, #0
	mov r0, r133
	sub sp, fp, #36
	add sp, sp, #4
	pop {r4-r10, fp, lr}
	bx lr

.balign 4
.global B$g
.section .text
.arm
B$g:
B$g$L100:
	push {r4-r10, fp, lr}
	sub sp, sp, #4
	add fp, sp, #36
	mov r100, r0
	movw r102, #300
	mov r0, r102
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
	movw r102, #100
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
