.balign 4
.global main
.section .text
.arm
main:
main$L106:
	push {r4-r10, fp, lr}
	sub sp, sp, #4
	add fp, sp, #36
	movw r0, #20
	bl malloc
	movw r1, #4
	str r1, [r0]
	movw r1, #1
	str r1, [r0, #4]
	movw r1, #2
	str r1, [r0, #8]
	movw r1, #3
	str r1, [r0, #12]
	movw r1, #4
	str r1, [r0, #16]
	ldr r1, [r0]
main$L101:
	movw r2, #2
	cmp r2, r1
	bge main$L100
main$L102:
	movw r1, #7
	str r1, [r0, #12]
	ldr r1, [r0]
main$L104:
	movw r2, #2
	cmp r2, r1
	bge main$L103
main$L105:
	ldr r0, [r0, #12]
	sub sp, fp, #36
	add sp, sp, #4
	pop {r4-r10, fp, lr}
	bx lr
main$L103:
	movw r0, #65535
	movt r0, #65535
	bl exit
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
