.balign 4
.global main
.section .text
.arm
main:
main$L100:
	push {r4-r10, fp, lr}
	sub sp, sp, #4
	add fp, sp, #36
	movw r106, #0
	mov r0, r106
	bl malloc
	mov r1030101, r0
	mov r1010101, r1030101
	mov r1000101, r1010101
	movw r107, #0
	mov r0, r107
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
