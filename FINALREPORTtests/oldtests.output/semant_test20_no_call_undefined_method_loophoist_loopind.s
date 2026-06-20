.balign 4
.global main
.section .text
.arm
main:
main$L100:
	push {r4-r10, fp, lr}
	sub sp, sp, #4
	add fp, sp, #36
	movw r110, #4
	mov r0, r110
	bl malloc
	mov r102010101, r0
	ldr r111, =A$f
	str r111, [r102010101]
	mov r100010101, r102010101
	ldr r104010101, [r100010101]
	mov r0, r100010101
	mov r1, r100010101
	mov r2, r100010101
	blx r104010101
	mov r105010101, r0
	mov r0, r105010101
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
	mov r101, r1
	mov r102, r2
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
