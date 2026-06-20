.balign 4
.global main
.section .text
.arm
main:
main$L103:
	push {r4-r10, fp, lr}
	sub sp, sp, #4
	add fp, sp, #36
	movw r1, #0
	movw r0, #123
	sub r5, r1, r0
	ldr r6, [r4]
	movw r0, #0
	cmp r5, r0
	bge main$L101
main$L100:
	movw r0, #65535
	movt r0, #65535
	bl exit
main$L101:
	cmp r5, r6
	bge main$L100
main$L102:
	add r0, r5, #1
	movw r1, #4
	mul r0, r0, r1
	movw r1, #4
	str r1, [r4, r0]
main$L104:
	movw r0, #0
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
