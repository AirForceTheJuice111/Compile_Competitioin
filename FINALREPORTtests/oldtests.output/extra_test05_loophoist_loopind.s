.balign 4
.global main
.section .text
.arm
main:
main$L100:
	push {r4-r10, fp, lr}
	sub sp, sp, #4
	add fp, sp, #36
	movw r105, #4
	movw r106, #5
	add r101010101, r105, r106
	add r102010101, r101010101, #3
	add r103010101, r102010101, #2
	add r104010101, r103010101, #1
	mov r0, r104010101
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
