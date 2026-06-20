.balign 4
.global main
.section .text
.arm
main:
main$L106:
	push {r4-r10, fp, lr}
	sub sp, sp, #4
	add fp, sp, #36
	movw r122, #16
	mov r0, r122
	bl malloc
	mov r1000101, r0
	movw r123, #0
	movw r124, #1
	sub r1010101, r123, r124
	movw r125, #3
	str r125, [r1000101]
	movw r126, #1
	str r126, [r1000101, #4]
	movw r127, #2
	str r127, [r1000101, #8]
	movw r128, #3
	str r128, [r1000101, #12]
	mov r1070101, r1000101
	ldr r1030101, [r1000101]
	movw r134, #0
	cmp r1010101, r134
	bge main$L101
main$L100:
	movw r135, #65535
	movt r135, #65535
	mov r0, r135
	bl exit
main$L101:
	cmp r1010101, r1030101
	bge main$L100
main$L102:
	add r1150101, r1010101, #1
	mov r1090101, r1000101
	movw r129, #4
	mul r1160101, r1150101, r129
	movw r130, #4
	str r130, [r1070101, r1160101]
	ldr r1040101, [r1000101]
	movw r136, #2
	movw r137, #0
	cmp r136, r137
	bge main$L104
main$L103:
	movw r138, #65535
	movt r138, #65535
	mov r0, r138
	bl exit
main$L104:
	movw r139, #2
	cmp r139, r1040101
	bge main$L103
main$L105:
	movw r131, #2
	movw r132, #1
	add r1190101, r131, r132
	movw r133, #4
	mul r1200101, r1190101, r133
	ldr r1010201, [r1090101, r1200101]
main$L107:
	movw r140, #0
	mov r0, r140
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
