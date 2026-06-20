.balign 4
.global main
.section .text
.arm
main:
main$L100:
	push {r4-r10, fp, lr}
	sub sp, sp, #4
	add fp, sp, #36
	mov r102010101, #3
	add r106010101, r102010101, #1
	movw r108, #4
	mul r104010101, r106010101, r108
	mov r0, r104010101
	bl malloc
	mov r103010101, r0
	str r102010101, [r103010101]
main$L101:
	movw r109, #0
	mov r0, r109
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
