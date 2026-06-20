.balign 4
.global main
.section .text
.arm
main:
main$L105:
	push {r4-r10, fp, lr}
	sub sp, sp, #4
	add fp, sp, #36
	bl getint
	mov r10301, r0
	mov r117, #0
	bl getint
	mov r10401, r0
	mov r10302, r10301
	mov r10202, r117
main$L102:
	movw r123, #0
	cmp r10302, r123
	bgt main$L103
main$L104:
	movw r122, #10
	mov r0, r122
	bl putch
	mov r0, r10202
	sub sp, fp, #36
	add sp, sp, #4
	pop {r4-r10, fp, lr}
	bx lr
main$L103:
	sub r10303, r10302, r10401
	movw r120, #8
	mul r11301, r120, r10303
	add r10203, r11301, #7
	mov r0, r10203
	bl putint
	movw r121, #32
	mov r0, r121
	bl putch
	mov r10302, r10303
	mov r10202, r10203
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
