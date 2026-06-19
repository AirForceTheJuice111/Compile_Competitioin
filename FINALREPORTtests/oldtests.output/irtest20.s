.balign 4
.global main
.section .text
.arm
main:
main$L100:
	push {r4-r10, fp, lr}
	sub sp, sp, #4
	add fp, sp, #36
	add r10901, r100, #4
	mov r11201, r101
	add r11501, r101, #8
	add r11801, r101, #12
	ldr r10801, [r100]
	mov r10001, r101
	mov r12601, r101
	add r12901, r101, #8
	add r13201, r101, #12
	ldr r11001, [r10901]
	ldr r11301, [r11201]
	ldr r11601, [r11501]
	ldr r11901, [r11801]
	add r11101, r10801, r11001
	mov r12101, r10001
	add r12301, r10001, #4
	add r11401, r11101, r11301
	add r11701, r11401, r11601
	add r10301, r11701, r11901
	mov r0, r10301
	bl putint
	ldr r12201, [r12101]
	ldr r12401, [r12301]
	ldr r12701, [r12601]
	ldr r13001, [r12901]
	ldr r13301, [r13201]
	add r12501, r12201, r12401
	add r12801, r12501, r12701
	add r13101, r12801, r13001
	add r10501, r13101, r13301
	mov r0, r10501
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
	ldr r10301, [r100]
	ldr r10501, [r100, #8]
	ldr r10801, [r100, #12]
	add r10601, r10301, r10501
	add r10901, r10601, r10801
	mov r0, r10901
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
