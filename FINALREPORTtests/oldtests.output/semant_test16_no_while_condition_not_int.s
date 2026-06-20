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
	mov r10201, r0
	mov r10001, r10201
main$L102:
	ldr r10601, [r10001]
	movw r108, #0
	cmp r10601, r108
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
