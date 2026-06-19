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
	mov r103010101, r0
	ldr r112, =A$f
	str r112, [r103010101]
	mov r100010101, r103010101
	ldr r105010101, [r100010101]
	mov r0, r100010101
	blx r105010101
	mov r101010101, r0
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
	mov r102010101, #1
	add r106010101, r102010101, #1
	movw r108, #4
	mul r104010101, r106010101, r108
	mov r0, r104010101
	bl malloc
	mov r103010101, r0
	str r102010101, [r103010101]
	mov r0, r103010101
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
