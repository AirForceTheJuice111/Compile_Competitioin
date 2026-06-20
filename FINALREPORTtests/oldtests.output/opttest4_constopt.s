.balign 4
.global main
.section .text
.arm
main:
main$L110:
	push {r4-r10, fp, lr}
	sub sp, sp, #4
	add fp, sp, #36
	mov r107, #10
	mov r108, #0
	bl getint
	mov r10101, r0
	mov r10302, r107
	mov r10201, r102
	mov r10102, r10101
	mov r10002, r108
main$L102:
	movw r112, #0
	cmp r10302, r112
	bgt main$L103
main$L104:
	mov r0, r10201
	sub sp, fp, #36
	add sp, sp, #4
	pop {r4-r10, fp, lr}
	bx lr
main$L103:
	add r10003, r10002, #1
	mov r10202, r10201
	mov r10103, r10102
main$L107:
	movw r113, #0
	cmp r10103, r113
	bgt main$L108
main$L109:
	sub r10303, r10302, #1
	mov r10302, r10303
	mov r10201, r10202
	mov r10102, r10103
	mov r10002, r10003
	b main$L102
main$L108:
	movw r111, #3
	mul r10203, r111, r10003
	sub r10104, r10103, #1
	mov r10202, r10203
	mov r10103, r10104
	b main$L107

.global malloc
.global getint
.global getch
.global getarray
.global putint
.global putch
.global putarray
.global starttime
.global stoptime
