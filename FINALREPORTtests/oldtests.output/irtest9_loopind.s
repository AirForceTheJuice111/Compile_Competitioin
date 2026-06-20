.balign 4
.global main
.section .text
.arm
main:
main$L100:
	push {r4-r10, fp, lr}
	sub sp, sp, #4
	add fp, sp, #36
	mov r1020101, #3
	add r1060101, r1020101, #1
	movw r108, #4
	mul r1040101, r1060101, r108
	mov r0, r1040101
	bl malloc
	mov r1030101, r0
	str r1020101, [r1030101]
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
