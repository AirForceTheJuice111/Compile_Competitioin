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
	mov r103010101, r0
	ldr r123, =A$f
	str r123, [r103010101, #8]
	mov r100010101, r103010101
	movw r124, #1
	str r124, [r100010101]
	mov r116010101, r100010101
	movw r125, #12
	mov r0, r125
	bl malloc
	mov r104010101, r0
	ldr r126, =B$f
	str r126, [r104010101, #8]
	mov r101010101, r104010101
	movw r127, #10
	str r127, [r101010101, #4]
	add r118010101, r101010101, #4
	ldr r117010101, [r116010101]
	ldr r119010101, [r118010101]
	add r107010101, r117010101, r119010101
	mov r0, r107010101
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

.global malloc
.global getint
.global getch
.global getarray
.global putint
.global putch
.global putarray
.global starttime
.global stoptime
