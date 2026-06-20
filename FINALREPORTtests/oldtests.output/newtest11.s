.balign 4
.global main
.section .text
.arm
main:
main$L108:
	push {r4-r10, fp, lr}
	sub sp, sp, #4
	add fp, sp, #36
	mov r10201, #0
	movw r141, #32
	mov r0, r141
	bl malloc
	mov r10001, r0
	movw r142, #7
	str r142, [r10001]
	movw r143, #6
	str r143, [r10001, #4]
	movw r144, #3
	str r144, [r10001, #8]
	movw r145, #0
	str r145, [r10001, #12]
	movw r146, #5
	str r146, [r10001, #16]
	movw r147, #9
	str r147, [r10001, #20]
	movw r148, #1
	str r148, [r10001, #24]
	movw r149, #2
	str r149, [r10001, #28]
	movw r150, #8
	mov r0, r150
	bl malloc
	mov r10401, r0
	mov r11101, r10001
	ldr r151, =b1$bubbleSort
	str r151, [r10401, #4]
	mov r10101, r10401
	ldr r11001, [r10101, #4]
	mov r11201, r10101
	ldr r10501, [r10001]
	mov r11301, r11001
	mov r0, r11201
	mov r1, r11101
	mov r2, r10501
	blx r11301
	mov r10202, r10201
main$L102:
	mov r11501, r10202
	ldr r10601, [r10001]
	cmp r11501, r10601
	blt main$L103
main$L104:
	movw r154, #10
	mov r0, r154
	bl putch
	movw r155, #0
	mov r0, r155
	sub sp, fp, #36
	add sp, sp, #4
	pop {r4-r10, fp, lr}
	bx lr
main$L103:
	mov r11701, r10001
	ldr r10701, [r10001]
	movw r156, #0
	cmp r10202, r156
	bge main$L106
main$L105:
	movw r157, #65535
	movt r157, #65535
	mov r0, r157
	bl exit
main$L106:
	cmp r10202, r10701
	bge main$L105
main$L107:
	add r13501, r10202, #1
	add r10203, r10202, #1
	movw r152, #4
	mul r13601, r13501, r152
	ldr r11801, [r11701, r13601]
	mov r0, r11801
	bl putint
	movw r153, #32
	mov r0, r153
	bl putch
	mov r10202, r10203
	b main$L102

.balign 4
.global b1$bubbleSort
.section .text
.arm
b1$bubbleSort:
b1$bubbleSort$L133:
	push {r4-r10, fp, lr}
	sub sp, sp, #4
	add fp, sp, #36
	mov r101, r0
	mov r102, r1
	mov r103, r2
	mov r10001, #0
	movw r172, #1
	cmp r103, r172
	ble b1$bubbleSort$L102
b1$bubbleSort$L103:
b1$bubbleSort$L104:
	mov r10002, r10001
b1$bubbleSort$L107:
	sub r13301, r103, #1
	cmp r10002, r13301
	blt b1$bubbleSort$L108
b1$bubbleSort$L109:
	ldr r12701, [r101, #4]
	mov r13001, r101
	mov r12901, r102
	sub r12801, r103, #1
	mov r13101, r12701
	mov r0, r13001
	mov r1, r12901
	mov r2, r12801
	blx r13101
	mov r13201, r0
	mov r0, r13201
	sub sp, fp, #36
	add sp, sp, #4
	pop {r4-r10, fp, lr}
	bx lr
b1$bubbleSort$L108:
	mov r11501, r102
	ldr r10501, [r102]
	movw r173, #0
	cmp r10002, r173
	bge b1$bubbleSort$L111
b1$bubbleSort$L110:
	movw r174, #65535
	movt r174, #65535
	mov r0, r174
	bl exit
b1$bubbleSort$L111:
	cmp r10002, r10501
	bge b1$bubbleSort$L110
b1$bubbleSort$L112:
	add r13501, r10002, #1
	add r10601, r10002, #1
	mov r11701, r102
	movw r166, #4
	mul r13601, r13501, r166
	ldr r11801, [r11501, r13601]
	ldr r10701, [r102]
	movw r175, #0
	cmp r10601, r175
	bge b1$bubbleSort$L114
b1$bubbleSort$L113:
	movw r176, #65535
	movt r176, #65535
	mov r0, r176
	bl exit
b1$bubbleSort$L114:
	cmp r10601, r10701
	bge b1$bubbleSort$L113
b1$bubbleSort$L115:
	add r13901, r10601, #1
	movw r167, #4
	mul r14001, r13901, r167
	ldr r14201, [r11701, r14001]
	cmp r11801, r14201
	bgt b1$bubbleSort$L130
b1$bubbleSort$L131:
b1$bubbleSort$L132:
	add r10003, r10002, #1
	mov r10002, r10003
	b b1$bubbleSort$L107
b1$bubbleSort$L130:
	mov r12001, r102
	ldr r10801, [r102]
	movw r177, #0
	cmp r10002, r177
	bge b1$bubbleSort$L119
b1$bubbleSort$L118:
	movw r178, #65535
	movt r178, #65535
	mov r0, r178
	bl exit
b1$bubbleSort$L119:
	cmp r10002, r10801
	bge b1$bubbleSort$L118
b1$bubbleSort$L120:
	mov r14401, r101
	add r14501, r10002, #1
	mov r12201, r102
	movw r168, #4
	mul r14601, r14501, r168
	ldr r14801, [r12001, r14601]
	str r14801, [r14401]
	ldr r10901, [r102]
	movw r179, #0
	cmp r10002, r179
	bge b1$bubbleSort$L122
b1$bubbleSort$L121:
	movw r180, #65535
	movt r180, #65535
	mov r0, r180
	bl exit
b1$bubbleSort$L122:
	cmp r10002, r10901
	bge b1$bubbleSort$L121
b1$bubbleSort$L123:
	add r11001, r10002, #1
	mov r12401, r102
	ldr r11101, [r102]
	movw r181, #0
	cmp r11001, r181
	bge b1$bubbleSort$L125
b1$bubbleSort$L124:
	movw r182, #65535
	movt r182, #65535
	mov r0, r182
	bl exit
b1$bubbleSort$L125:
	cmp r11001, r11101
	bge b1$bubbleSort$L124
b1$bubbleSort$L126:
	add r15101, r10002, #1
	add r15401, r11001, #1
	add r11201, r10002, #1
	mov r12601, r102
	movw r169, #4
	mul r15201, r15101, r169
	movw r170, #4
	mul r15501, r15401, r170
	add r15301, r12201, r15201
	ldr r15701, [r12401, r15501]
	str r15701, [r15301]
	ldr r11301, [r102]
	movw r183, #0
	cmp r11201, r183
	bge b1$bubbleSort$L128
b1$bubbleSort$L127:
	movw r184, #65535
	movt r184, #65535
	mov r0, r184
	bl exit
b1$bubbleSort$L128:
	cmp r11201, r11301
	bge b1$bubbleSort$L127
b1$bubbleSort$L129:
	add r15901, r11201, #1
	ldr r16301, [r101]
	movw r171, #4
	mul r16001, r15901, r171
	str r16301, [r12601, r16001]
	b b1$bubbleSort$L132
b1$bubbleSort$L102:
	movw r185, #0
	mov r0, r185
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
