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
	mov r10201, #10
	mov r10101, #0
	mov r10104, #4
	movw r115, #3
	mul r10109, r10201, r115
	add r10107, r10109, #2
	mov r10102, r10101
	mov r10106, r10107
main$L102:
	movw r118, #2
	cmp r10106, r118
	bgt main$L103
main$L104:
	mov r0, r10102
	bl putint
	movw r117, #10
	mov r0, r117
	bl putch
	mov r0, r10102
	sub sp, fp, #36
	add sp, sp, #4
	pop {r4-r10, fp, lr}
	bx lr
main$L103:
	movw r119, #0
	cmp r10001, r119
	bgt main$L107
main$L108:
	mov r10105, r10104
main$L109:
	sub r10108, r10106, #3
	mov r10102, r10105
	mov r10106, r10108
	b main$L102
main$L107:
	mov r0, r10106
	bl putint
	movw r116, #32
	mov r0, r116
	bl putch
	mov r10105, r10106
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
