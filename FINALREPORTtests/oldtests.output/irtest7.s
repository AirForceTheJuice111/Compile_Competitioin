.balign 4
.global main
.section .text
.arm
main:
main$L108:
	push {r4-r10, fp, lr}
	sub sp, sp, #4
	add fp, sp, #36
	mov r10001, #1
	mov r10101, #2
	bl getint
	mov r10401, r0
	mov r10301, #0
	mov r10501, r10401
	movw r108, #1
	cmp r10001, r108
	bgt main$L105
main$L104:
	movw r109, #2
	cmp r10101, r109
	blt main$L105
	mov r10303, r10301
main$L106:
	add r10701, r10501, r10303
	mov r0, r10701
	sub sp, fp, #36
	add sp, sp, #4
	pop {r4-r10, fp, lr}
	bx lr
main$L105:
	mov r10302, #1
	mov r10303, r10302
	b main$L106

.global malloc
.global getint
.global getch
.global getarray
.global putint
.global putch
.global putarray
.global starttime
.global stoptime
