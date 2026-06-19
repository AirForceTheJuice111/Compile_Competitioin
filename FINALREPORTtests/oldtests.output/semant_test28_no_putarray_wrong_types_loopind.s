.balign 4
.global main
.section .text
.arm
main:
main$L100:
	push {r4-r10, fp, lr}
	sub sp, sp, #4
	add fp, sp, #36
	mov r1020101, #2
	add r1070101, r1020101, #1
	movw r110, #4
	mul r1040101, r1070101, r110
	mov r0, r1040101
	bl malloc
	mov r1030101, r0
	str r1020101, [r1030101]
	mov r1000101, r1030101
	movw r111, #1
	mov r0, r111
	mov r1, r1000101
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
