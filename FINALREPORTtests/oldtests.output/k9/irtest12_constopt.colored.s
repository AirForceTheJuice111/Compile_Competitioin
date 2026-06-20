.balign 4
.global main
.section .text
.arm
main:
main$L106:
	push {r4-r10, fp, lr}
	sub sp, sp, #4
	add fp, sp, #36
	movw r0, #16
	bl malloc
	movw r1, #3
	str r1, [r0]
	movw r1, #1
	str r1, [r0, #4]
	movw r1, #2
	str r1, [r0, #8]
	movw r1, #3
	str r1, [r0, #12]
	ldr r0, [r0]
main$L100:
	movw r0, #65535
	movt r0, #65535
	bl exit

.global malloc
.global getint
.global getch
.global getarray
.global putint
.global putch
.global putarray
.global starttime
.global stoptime
