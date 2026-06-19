.balign 4
.global main
.section .text
.arm
main:
main$L110:
	push {r4-r10, fp, lr}
	sub sp, sp, #4
	add fp, sp, #36
main$L107:
main$L104:
	mov r10901, #1
main$L105:
	mov r1030301, r10901
	add r1040101, r1030301, #1
	mov r0, r1040101
	bl putint
main$L109:
main$L111:
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
