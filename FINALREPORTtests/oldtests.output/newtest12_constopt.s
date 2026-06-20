.balign 4
.global fib$f
.section .text
.arm
fib$f:
fib$f$L108:
	push {r4-r10, fp, lr}
	sub sp, sp, #4
	add fp, sp, #36
	mov r100, r0
	mov r101, r1
	movw r121, #0
	cmp r101, r121
	beq fib$f$L105
fib$f$L104:
	movw r122, #1
	cmp r101, r122
	beq fib$f$L105
fib$f$L106:
	ldr r10301, [r100]
	mov r10501, r100
	sub r10401, r101, #1
	mov r11601, r100
	mov r11001, r100
	sub r10901, r101, #2
	mov r10601, r10301
	mov r0, r10501
	mov r1, r10401
	blx r10601
	mov r10701, r0
	ldr r10801, [r11601]
	mov r11301, r10701
	mov r11101, r10801
	mov r0, r11001
	mov r1, r10901
	blx r11101
	mov r11201, r0
	add r11801, r11301, r11201
	mov r0, r11801
	sub sp, fp, #36
	add sp, sp, #4
	pop {r4-r10, fp, lr}
	bx lr
fib$f$L105:
	mov r0, r101
	sub sp, fp, #36
	add sp, sp, #4
	pop {r4-r10, fp, lr}
	bx lr

.balign 4
.global main
.section .text
.arm
main:
main$L113:
	push {r4-r10, fp, lr}
	sub sp, sp, #4
	add fp, sp, #36
	movw r174, #4
	mov r0, r174
	bl malloc
	mov r10401, r0
	ldr r175, =fib$f
	str r175, [r10401]
	mov r10201, r10401
	movw r176, #69
	mov r0, r176
	bl putch
	movw r177, #110
	mov r0, r177
	bl putch
	movw r178, #116
	mov r0, r178
	bl putch
	movw r179, #101
	mov r0, r179
	bl putch
	movw r180, #114
	mov r0, r180
	bl putch
	movw r181, #32
	mov r0, r181
	bl putch
	movw r182, #116
	mov r0, r182
	bl putch
	movw r183, #104
	mov r0, r183
	bl putch
	movw r184, #101
	mov r0, r184
	bl putch
	movw r185, #32
	mov r0, r185
	bl putch
	movw r186, #110
	mov r0, r186
	bl putch
	movw r187, #117
	mov r0, r187
	bl putch
	movw r188, #109
	mov r0, r188
	bl putch
	movw r189, #98
	mov r0, r189
	bl putch
	movw r190, #101
	mov r0, r190
	bl putch
	movw r191, #114
	mov r0, r191
	bl putch
	movw r192, #32
	mov r0, r192
	bl putch
	movw r193, #111
	mov r0, r193
	bl putch
	movw r194, #102
	mov r0, r194
	bl putch
	movw r195, #32
	mov r0, r195
	bl putch
	movw r196, #116
	mov r0, r196
	bl putch
	movw r197, #101
	mov r0, r197
	bl putch
	movw r198, #114
	mov r0, r198
	bl putch
	movw r199, #109
	mov r0, r199
	bl putch
	movw r200, #58
	mov r0, r200
	bl putch
	bl getint
	mov r10101, r0
	movw r203, #0
	cmp r10101, r203
	blt main$L105
main$L104:
	movw r204, #47
	cmp r10101, r204
	bgt main$L105
main$L106:
main$L107:
	mov r171, #0
	mov r10002, r171
main$L110:
	cmp r10002, r10101
	blt main$L111
main$L112:
	movw r202, #10
	mov r0, r202
	bl putch
	movw r205, #0
	mov r0, r205
	sub sp, fp, #36
	add sp, sp, #4
	pop {r4-r10, fp, lr}
	bx lr
main$L111:
	ldr r13201, [r10201]
	add r10003, r10002, #1
	mov r0, r10201
	mov r1, r10002
	blx r13201
	mov r13301, r0
	mov r0, r13301
	bl putint
	movw r201, #32
	mov r0, r201
	bl putch
	mov r10002, r10003
	b main$L110
main$L105:
	movw r206, #65535
	movt r206, #65535
	mov r0, r206
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
