.balign 4
.global main
.section .text
.arm
main:
main$L116:
	push {r4-r10, fp, lr}
	sub sp, sp, #4
	add fp, sp, #36
	mov r1000101, #0
	mov r1030101, #100
	add r1190101, r1030101, #1
	movw r132, #4
	mul r1090101, r1190101, r132
	mov r0, r1090101
	bl malloc
	mov r1040101, r0
	str r1030101, [r1040101]
	mov r1010101, r1040101
main$L102:
	mov r1110101, r1000101
	ldr r1050101, [r1010101]
	cmp r1110101, r1050101
	blt main$L103
main$L104:
	ldr r1070101, [r1010101]
	movw r138, #99
	cmp r1070101, r138
	bgt main$L113
main$L114:
main$L115:
	movw r137, #10
	mov r0, r137
	bl putch
	movw r139, #1
	mov r0, r139
	sub sp, fp, #36
	add sp, sp, #4
	pop {r4-r10, fp, lr}
	bx lr
main$L113:
	mov r1150101, r1010101
	ldr r1080101, [r1010101]
	movw r140, #99
	movw r141, #0
	cmp r140, r141
	bge main$L111
main$L110:
	movw r142, #65535
	movt r142, #65535
	mov r0, r142
	bl exit
main$L111:
	movw r143, #99
	cmp r143, r1080101
	bge main$L110
main$L112:
	movw r134, #99
	movw r135, #1
	add r1270101, r134, r135
	movw r136, #4
	mul r1280101, r1270101, r136
	ldr r1160101, [r1150101, r1280101]
	mov r0, r1160101
	bl putint
	b main$L115
main$L103:
	mov r1130101, r1010101
	ldr r1060101, [r1010101]
	movw r144, #0
	cmp r1000101, r144
	bge main$L106
main$L105:
	movw r145, #65535
	movt r145, #65535
	mov r0, r145
	bl exit
main$L106:
	cmp r1000101, r1060101
	bge main$L105
main$L107:
	add r1220101, r1000101, #1
	mul r1250101, r1000101, r1000101
	movw r133, #4
	mul r1230101, r1220101, r133
	str r1250101, [r1130101, r1230101]
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
