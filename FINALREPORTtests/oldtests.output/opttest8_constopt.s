.balign 4
.global main
.section .text
.arm
main:
main$L110:
	push {r4-r10, fp, lr}
	sub sp, sp, #4
	add fp, sp, #36
	mov r104, #0
	mov r105, #0
	mov r10102, r104
	mov r10002, r105
main$L102:
	movw r109, #10
	cmp r10002, r109
	blt main$L103
main$L104:
	mov r0, r10102
	sub sp, fp, #36
	add sp, sp, #4
	pop {r4-r10, fp, lr}
	bx lr
main$L103:
	movw r110, #5
	cmp r10002, r110
	bgt main$L107
main$L108:
	mov r10104, r10102
main$L109:
	add r10003, r10002, #1
	mov r10102, r10104
	mov r10002, r10003
	b main$L102
main$L107:
	mov r106, #300
	mov r10104, r106
	b main$L109

.global malloc
.global getint
.global getch
.global getarray
.global putint
.global putch
.global putarray
.global starttime
.global stoptime
