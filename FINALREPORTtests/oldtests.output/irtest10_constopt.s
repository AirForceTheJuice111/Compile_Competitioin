.balign 4
.global main
.section .text
.arm
main:
main$L100:
	push {r4-r10, fp, lr}
	sub sp, sp, #4
	add fp, sp, #36
	movw r109, #16
	mov r0, r109
	bl malloc
	mov r1000101, r0
	movw r110, #3
	str r110, [r1000101]
	movw r111, #1
	str r111, [r1000101, #4]
	movw r112, #2
	str r112, [r1000101, #8]
	movw r113, #3
	str r113, [r1000101, #12]
main$L101:
	movw r114, #0
	mov r0, r114
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
