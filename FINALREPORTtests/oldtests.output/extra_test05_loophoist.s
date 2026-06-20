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
	add r1010101, r105, r106
	add r1020101, r1010101, #3
	add r1030101, r1020101, #2
	add r1040101, r1030101, #1
	mov r0, r1040101
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
