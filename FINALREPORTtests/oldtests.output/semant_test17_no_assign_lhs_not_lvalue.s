.balign 4
.global main
.section .text
.arm
main:
main$L103:
	push {r4-r10, fp, lr}
	sub sp, sp, #4
	add fp, sp, #36
	mov r10101, #1
	add r10901, r10101, #1
	movw r115, #4
	mul r10501, r10901, r115
	mov r0, r10501
	bl malloc
	mov r10201, r0
	str r10101, [r10201]
	mov r10301, r10201
	ldr r10401, [r10301]
	mov r10801, r10301
	movw r120, #0
	movw r121, #0
	cmp r120, r121
	bge main$L101
main$L100:
	movw r122, #65535
	movt r122, #65535
	mov r0, r122
	bl exit
main$L101:
	movw r123, #0
	cmp r123, r10401
	bge main$L100
main$L102:
	movw r116, #0
	movw r117, #1
	add r11201, r116, r117
	movw r118, #4
	mul r11301, r11201, r118
	movw r119, #2
	str r119, [r10801, r11301]
	movw r124, #0
	mov r0, r124
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
