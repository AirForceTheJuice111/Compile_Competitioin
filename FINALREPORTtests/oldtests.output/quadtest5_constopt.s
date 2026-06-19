.balign 4
.global main
.section .text
.arm
main:
main$L110:
	push {r4-r10, fp, lr}
	sub sp, sp, #4
	add fp, sp, #36
main$L102:
main$L103:
main$L107:
main$L104:
	movw r109, #100
	mov r0, r109
	bl putint
	movw r110, #10
	mov r0, r110
	bl putch
	movw r111, #1
	mov r0, r111
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
