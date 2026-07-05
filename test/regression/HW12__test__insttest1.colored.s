.balign 4
.global main
.section .text
.arm
main:
main$L108:
	push {r4-r10, fp, lr}
	sub sp, sp, #4
	add fp, sp, #36
	movw r0, #20
	push {r0}
	pop {r0}
	bl malloc
	mov r2, r0
	movw r1, #4
	str r1, [r2]
	movw r0, #1
	str r0, [r2, #4]
	movw r0, #2
	str r0, [r2, #8]
	movw r0, #3
	str r0, [r2, #12]
	str r1, [r2, #16]
	ldr r0, [r2]

.global malloc
.global getint
.global getch
.global getarray
.global putint
.global putch
.global putarray
.global starttime
.global stoptime
