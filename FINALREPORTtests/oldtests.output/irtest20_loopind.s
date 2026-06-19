.balign 4
.global main
.section .text
.arm
main:
main$L100:
	push {r4-r10, fp, lr}
	sub sp, sp, #4
	add fp, sp, #36
	add r1090101, r100, #4
	mov r1120101, r101
	add r1150101, r101, #8
	add r1180101, r101, #12
	ldr r1080101, [r100]
	mov r1000101, r101
	mov r1260101, r101
	add r1290101, r101, #8
	add r1320101, r101, #12
	ldr r1100101, [r1090101]
	ldr r1130101, [r1120101]
	ldr r1160101, [r1150101]
	ldr r1190101, [r1180101]
	add r1110101, r1080101, r1100101
	mov r1210101, r1000101
	add r1230101, r1000101, #4
	add r1140101, r1110101, r1130101
	add r1170101, r1140101, r1160101
	add r1030101, r1170101, r1190101
	mov r0, r1030101
	bl putint
	ldr r1220101, [r1210101]
	ldr r1240101, [r1230101]
	ldr r1270101, [r1260101]
	ldr r1300101, [r1290101]
	ldr r1330101, [r1320101]
	add r1250101, r1220101, r1240101
	add r1280101, r1250101, r1270101
	add r1310101, r1280101, r1300101
	add r1050101, r1310101, r1330101
	mov r0, r1050101
	bl putint
main$L101:
	movw r135, #0
	mov r0, r135
	sub sp, fp, #36
	add sp, sp, #4
	pop {r4-r10, fp, lr}
	bx lr

.balign 4
.global B$f
.section .text
.arm
B$f:
B$f$L100:
	push {r4-r10, fp, lr}
	sub sp, sp, #4
	add fp, sp, #36
	mov r100, r0
	ldr r1030101, [r100]
	ldr r1050101, [r100, #8]
	ldr r1080101, [r100, #12]
	add r1060101, r1030101, r1050101
	add r1090101, r1060101, r1080101
	mov r0, r1090101
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
