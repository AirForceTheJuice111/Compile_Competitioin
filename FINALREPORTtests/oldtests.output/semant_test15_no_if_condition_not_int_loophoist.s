.balign 4
.global main
.section .text
.arm
main:
main$L108:
	push {r4-r10, fp, lr}
	sub sp, sp, #4
	add fp, sp, #36
	mov r1020101, #2
	add r1090101, r1020101, #1
	movw r116, #4
	mul r1050101, r1090101, r116
	mov r0, r1050101
	bl malloc
	mov r1030101, r0
	str r1020101, [r1030101]
	mov r1000101, r1030101
	ldr r1040101, [r1000101]
	mov r1080101, r1000101
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
	cmp r123, r1040101
	bge main$L100
main$L102:
	movw r117, #0
	movw r118, #1
	add r1120101, r117, r118
	movw r119, #4
	mul r1130101, r1120101, r119
	ldr r1150101, [r1080101, r1130101]
	movw r124, #0
	cmp r1150101, r124
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
