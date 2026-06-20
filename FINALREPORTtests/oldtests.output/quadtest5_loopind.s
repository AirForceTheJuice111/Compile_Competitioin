.balign 4
.global main
.section .text
.arm
main:
main$L110:
	push {r4-r10, fp, lr}
	sub sp, sp, #4
	add fp, sp, #36
	mov r1000101, #1
	mov r1010101, #2
main$L102:
	movw r109, #0
	cmp r1000101, r109
	bne main$L103
main$L104:
	movw r107, #100
	mov r0, r107
	bl putint
	movw r108, #10
	mov r0, r108
	bl putch
	movw r110, #1
	mov r0, r110
	sub sp, fp, #36
	add sp, sp, #4
	pop {r4-r10, fp, lr}
	bx lr
main$L103:
	movw r111, #0
	cmp r1010101, r111
	bne main$L107
main$L108:
	b main$L102
main$L107:
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
