.balign 4
.global main
.section .text
.arm
main:
main$L110:
	push {r4-r10, fp, lr}
	sub sp, sp, #4
	add fp, sp, #36
	mov r10001, #1
	mov r10101, #2
main$L102:
	movw r103, #0
	cmp r10001, r103
	bne main$L103
main$L104:
main$L112:
	movw r104, #0
	mov r0, r104
	sub sp, fp, #36
	add sp, sp, #4
	pop {r4-r10, fp, lr}
	bx lr
main$L103:
	movw r105, #0
	cmp r10101, r105
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
