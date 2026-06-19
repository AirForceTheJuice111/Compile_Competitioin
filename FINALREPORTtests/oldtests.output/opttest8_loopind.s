.balign 4
.global main
.section .text
.arm
main:
main$L110:
	push {r4-r10, fp, lr}
	sub sp, sp, #4
	add fp, sp, #36
	mov r10001, #0
	mov r10101, #0
	mov r10102, r10101
	mov r10002, r10001
main$L102:
	movw r106, #10
	cmp r10002, r106
	blt main$L103
main$L104:
	mov r0, r10102
	sub sp, fp, #36
	add sp, sp, #4
	pop {r4-r10, fp, lr}
	bx lr
main$L103:
	movw r107, #5
	cmp r10002, r107
	bgt main$L107
main$L108:
	mov r10104, r10102
main$L109:
	add r10003, r10002, #1
	mov r10102, r10104
	mov r10002, r10003
	b main$L102
main$L107:
	movw r104, #100
	movw r105, #200
	add r10202, r104, r105
	mov r10103, r10202
	mov r10104, r10103
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
