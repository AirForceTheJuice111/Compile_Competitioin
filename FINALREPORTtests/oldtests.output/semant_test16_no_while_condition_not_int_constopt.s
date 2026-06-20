.balign 4
.global main
.section .text
.arm
main:
main$L105:
	push {r4-r10, fp, lr}
	sub sp, sp, #4
	add fp, sp, #36
	movw r109, #4
	mov r0, r109
	bl malloc
	mov r1020101, r0
	mov r1000101, r1020101
main$L102:
	ldr r1060101, [r1000101]
	movw r110, #0
	cmp r1060101, r110
	bne main$L103
main$L104:
	movw r111, #0
	mov r0, r111
	sub sp, fp, #36
	add sp, sp, #4
	pop {r4-r10, fp, lr}
	bx lr
main$L103:
	b main$L102

.global malloc
.global getint
.global getch
.global getarray
.global putint
.global putch
.global putarray
.global starttime
.global stoptime
