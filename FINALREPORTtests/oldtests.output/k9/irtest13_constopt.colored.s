.balign 4
.global main
.section .text
.arm
main:
main$L103:
	push {r4-r10, fp, lr}
	sub sp, sp, #4
	add fp, sp, #36
	movw r0, #8
	bl malloc
	movw r1, #1
	str r1, [r0]
	ldr r1, [r0]
main$L101:
	movw r2, #0
	cmp r2, r1
	bge main$L100
main$L102:
	movw r1, #0
	str r1, [r0, #4]
main$L104:
	movw r0, #0
	sub sp, fp, #36
	add sp, sp, #4
	pop {r4-r10, fp, lr}
	bx lr
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
