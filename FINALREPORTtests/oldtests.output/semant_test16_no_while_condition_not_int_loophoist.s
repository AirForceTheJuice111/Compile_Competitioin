.balign 4
.global main
.section .text
.arm
main:
main$L105:
	push {r4-r10, fp, lr}
	sub sp, sp, #4
	add fp, sp, #36
	movw r107, #4
	mov r0, r107
	bl malloc
	mov r1020101, r0
	mov r1000101, r1020101
	mov r1050101, r1000101
main$L102:
	ldr r1060101, [r1050101]
	movw r108, #0
	cmp r1060101, r108
	bne main$L103
main$L104:
	movw r109, #0
	mov r0, r109
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
