.balign 4
.global main
.section .text
.arm
main:
main$L116:
	push {r4-r10, fp, lr}
	sub sp, sp, #4
	add fp, sp, #36
	movw r134, #404
	mov r0, r134
	bl malloc
	mov r1040101, r0
	movw r135, #100
	str r135, [r1040101]
	mov r1010101, r1040101
main$L102:
	ldr r1050101, [r1010101]
	movw r138, #0
	cmp r138, r1050101
	blt main$L103
main$L104:
	ldr r1070101, [r1010101]
	movw r139, #99
	cmp r1070101, r139
	bgt main$L113
main$L114:
main$L115:
	movw r137, #10
	mov r0, r137
	bl putch
	movw r140, #1
	mov r0, r140
	sub sp, fp, #36
	add sp, sp, #4
	pop {r4-r10, fp, lr}
	bx lr
main$L113:
	mov r1150101, r1010101
	ldr r1080101, [r1010101]
main$L111:
	movw r141, #99
	cmp r141, r1080101
	bge main$L110
main$L112:
	ldr r1160101, [r1150101, #400]
	mov r0, r1160101
	bl putint
	b main$L115
main$L110:
	movw r142, #65535
	movt r142, #65535
	mov r0, r142
	bl exit
main$L103:
	mov r1130101, r1010101
	ldr r1060101, [r1010101]
main$L106:
	movw r143, #0
	cmp r143, r1060101
	bge main$L105
main$L107:
	movw r136, #0
	str r136, [r1130101, #4]
	b main$L102
main$L105:
	movw r144, #65535
	movt r144, #65535
	mov r0, r144
	bl exit

.global malloc
.global getint
.global getch
.global getarray
.global putint
.global putch
.global putarray
.global starttime
.global stoptime
