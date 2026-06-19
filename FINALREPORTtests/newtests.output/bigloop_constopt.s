.balign 4
.global main
.section .text
.arm
main:
main$L110:
	push {r4-r10, fp, lr}
	sub sp, sp, #4
	add fp, sp, #36
	mov r116, #0
	mov r115, #0
	mov r10202, r115
	mov r10002, r116
main$L102:
	movw r124, #6000
	cmp r10202, r124
	blt main$L103
main$L104:
	mov r0, r10002
	bl putint
	movw r123, #10
	mov r0, r123
	bl putch
	movw r125, #0
	mov r0, r125
	sub sp, fp, #36
	add sp, sp, #4
	pop {r4-r10, fp, lr}
	bx lr
main$L103:
	add r10203, r10202, #1
	mov r117, #0
	mov r10302, r117
	mov r10003, r10002
main$L107:
	movw r120, #2000
	mul r10901, r10203, r120
	cmp r10302, r10901
	blt main$L108
main$L109:
	mov r10202, r10203
	mov r10002, r10003
	b main$L102
main$L108:
	add r10303, r10302, #1
	movw r121, #3
	mul r10101, r10203, r121
	mul r11001, r10303, r10101
	movw r122, #17
	sdiv r11201, r10303, r122
	add r11101, r10003, r11001
	add r10004, r11101, r11201
	mov r10302, r10303
	mov r10003, r10004
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
