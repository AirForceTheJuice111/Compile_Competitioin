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
	movw r118, #0
	cmp r10001, r118
	bgt main$L107
main$L108:
	mov r10104, #4
	mov r10105, r10104
main$L109:
	mov r0, r10105
	bl putint
	movw r117, #10
	mov r0, r117
	bl putch
	mov r0, r10105
	sub sp, fp, #36
	add sp, sp, #4
	pop {r4-r10, fp, lr}
	bx lr
main$L107:
	movw r115, #3
	mul r10109, r10201, r115
	add r10107, r10109, #2
	mov r10102, r10101
	mov r10106, r10107
main$L104:
	movw r119, #2
	cmp r10106, r119
	bgt main$L105
main$L106:
	mov r10105, r10102
	b main$L109
main$L105:
	mov r0, r10106
	bl putint
	sub r10108, r10106, #3
	movw r116, #32
	mov r0, r116
	bl putch
	mov r10102, r10106
	mov r10106, r10108
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
