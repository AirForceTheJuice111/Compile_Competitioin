.balign 4
.global main
.section .text
.arm
main:
main$L110:
	push {r4-r10, fp, lr}
	sub sp, sp, #4
	add fp, sp, #36
	movw r10401, #3000
	mov r10001, #0
	mov r10301, #0
	mov r10201, #0
	movw r115, #2
	mul r10801, r10401, r115
	mov r10202, r10201
	mov r10002, r10001
main$L102:
	cmp r10202, r10801
	blt main$L103
main$L104:
	mov r0, r10002
	bl putint
	movw r119, #10
	mov r0, r119
	bl putch
	movw r120, #0
	mov r0, r120
	sub sp, fp, #36
	add sp, sp, #4
	pop {r4-r10, fp, lr}
	bx lr
main$L103:
	add r10203, r10202, #1
	movw r116, #2000
	mul r10901, r10203, r116
	movw r117, #3
	mul r10101, r10203, r117
	mov r10302, r10301
	mov r10003, r10002
main$L107:
	cmp r10302, r10901
	blt main$L108
main$L109:
	mov r10202, r10203
	mov r10002, r10003
	b main$L102
main$L108:
	add r10303, r10302, #1
	mul r11001, r10303, r10101
	movw r118, #17
	sdiv r11201, r10303, r118
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
