.balign 4
.global main
.section .text
.arm
main:
main$L108:
	push {r4-r10, fp, lr}
	sub sp, sp, #4
	add fp, sp, #36
	mov r10201, #2
	add r10901, r10201, #1
	movw r116, #4
	mul r10501, r10901, r116
	mov r0, r10501
	bl malloc
	mov r10301, r0
	str r10201, [r10301]
	mov r10001, r10301
	ldr r10401, [r10001]
	mov r10801, r10001
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
	movw r117, #0
	movw r118, #1
	add r11201, r117, r118
	movw r119, #4
	mul r11301, r11201, r119
	ldr r11501, [r10801, r11301]
	movw r124, #0
	cmp r11501, r124
	bne main$L105
main$L106:
main$L107:
	movw r125, #0
	mov r0, r125
	sub sp, fp, #36
	add sp, sp, #4
	pop {r4-r10, fp, lr}
	bx lr
main$L105:
	movw r126, #1
	mov r0, r126
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
