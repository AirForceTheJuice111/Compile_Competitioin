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
	mov r115, #10
	mov r116, #0
	mov r10202, r115
	mov r10102, r116
main$L102:
	movw r123, #0
	cmp r10202, r123
	bgt main$L103
main$L104:
	mov r0, r10102
	bl putint
	movw r122, #10
	mov r0, r122
	bl putch
	mov r0, r10102
	sub sp, fp, #36
	add sp, sp, #4
	pop {r4-r10, fp, lr}
	bx lr
main$L103:
	movw r124, #0
	cmp r10001, r124
	bgt main$L107
main$L108:
	mov r117, #4
	mov r10105, r117
main$L109:
	sub r10203, r10202, #1
	mov r10202, r10203
	mov r10102, r10105
	b main$L102
main$L107:
	movw r120, #3
	mul r11001, r120, r10202
	add r10103, r11001, #2
	mov r0, r10103
	bl putint
	movw r121, #32
	mov r0, r121
	bl putch
	mov r10105, r10103
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
