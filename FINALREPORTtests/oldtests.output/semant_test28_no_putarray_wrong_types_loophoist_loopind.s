.balign 4
.global main
.section .text
.arm
main:
main$L100:
	push {r4-r10, fp, lr}
	sub sp, sp, #4
	add fp, sp, #36
	mov r102010101, #2
	add r107010101, r102010101, #1
	movw r110, #4
	mul r104010101, r107010101, r110
	mov r0, r104010101
	bl malloc
	mov r103010101, r0
	str r102010101, [r103010101]
	mov r100010101, r103010101
	movw r111, #1
	mov r0, r111
	mov r1, r100010101
	bl putarray
	movw r112, #0
	mov r0, r112
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
