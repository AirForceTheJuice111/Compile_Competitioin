.balign 4
.global main
.section .text
.arm
main:
main$L110:
	push {r4-r10, fp, lr}
	sub sp, sp, #4
	add fp, sp, #36
	bl getint
	mov r10001, r0
	mov r106, #0
	mov r10102, r106
	mov r10002, r10001
main$L102:
	movw r109, #0
	cmp r10002, r109
	bgt main$L103
main$L104:
	mov r0, r10102
	sub sp, fp, #36
	add sp, sp, #4
	pop {r4-r10, fp, lr}
	bx lr
main$L103:
	movw r110, #0
	cmp r10102, r110
	bne main$L107
main$L108:
main$L109:
	sub r10003, r10002, #1
	mov r105, #4
	mov r10102, r105
	mov r10002, r10003
	b main$L102
main$L107:
	mov r0, r10102
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
