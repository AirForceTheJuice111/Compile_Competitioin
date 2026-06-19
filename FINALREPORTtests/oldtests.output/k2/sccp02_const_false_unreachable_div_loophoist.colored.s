.balign 4
.global main
.section .text
.arm
main:
main$L105:
	push {r4-r10, fp, lr}
	sub sp, sp, #8
	add fp, sp, #40
	mov r10, #0
	str r10, [fp, #-40]
	movw r1, #0
	movw r0, #0
	cmp r1, r0
	bne main$L102
main$L103:
	mov r0, #7
main$L104:
	sub sp, fp, #40
	add sp, sp, #8
	pop {r4-r10, fp, lr}
	bx lr
main$L102:
	movw r0, #10
	ldr r10, [fp, #-40]
	sdiv r0, r0, r10
	b main$L104

.global malloc
.global getint
.global getch
.global getarray
.global putint
.global putch
.global putarray
.global starttime
.global stoptime
