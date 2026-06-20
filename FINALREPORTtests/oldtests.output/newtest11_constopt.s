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
	movw r175, #1
	cmp r103, r175
	ble b1$bubbleSort$L102
b1$bubbleSort$L103:
b1$bubbleSort$L104:
	mov r166, #0
	mov r10002, r166
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
	movw r176, #0
	cmp r10002, r176
	bge b1$bubbleSort$L111
b1$bubbleSort$L110:
	movw r177, #65535
	movt r177, #65535
	mov r0, r177
	bl exit
b1$bubbleSort$L111:
	cmp r10002, r10501
	bge b1$bubbleSort$L110
b1$bubbleSort$L112:
	add r13501, r10002, #1
	add r10601, r10002, #1
	mov r11701, r102
	movw r169, #4
	mul r13601, r13501, r169
	ldr r11801, [r11501, r13601]
	ldr r10701, [r102]
	movw r178, #0
	cmp r10601, r178
	bge b1$bubbleSort$L114
b1$bubbleSort$L113:
	movw r179, #65535
	movt r179, #65535
	mov r0, r179
	bl exit
b1$bubbleSort$L114:
	cmp r10601, r10701
	bge b1$bubbleSort$L113
b1$bubbleSort$L115:
	add r13901, r10601, #1
	movw r170, #4
	mul r14001, r13901, r170
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
	movw r180, #0
	cmp r10002, r180
	bge b1$bubbleSort$L119
b1$bubbleSort$L118:
	movw r181, #65535
	movt r181, #65535
	mov r0, r181
	bl exit
b1$bubbleSort$L119:
	cmp r10002, r10801
	bge b1$bubbleSort$L118
b1$bubbleSort$L120:
	mov r14401, r101
	add r14501, r10002, #1
	mov r12201, r102
	movw r171, #4
	mul r14601, r14501, r171
	ldr r14801, [r12001, r14601]
	str r14801, [r14401]
	ldr r10901, [r102]
	movw r182, #0
	cmp r10002, r182
	bge b1$bubbleSort$L122
b1$bubbleSort$L121:
	movw r183, #65535
	movt r183, #65535
	mov r0, r183
	bl exit
b1$bubbleSort$L122:
	cmp r10002, r10901
	bge b1$bubbleSort$L121
b1$bubbleSort$L123:
	add r11001, r10002, #1
	mov r12401, r102
	ldr r11101, [r102]
	movw r184, #0
	cmp r11001, r184
	bge b1$bubbleSort$L125
b1$bubbleSort$L124:
	movw r185, #65535
	movt r185, #65535
	mov r0, r185
	bl exit
b1$bubbleSort$L125:
	cmp r11001, r11101
	bge b1$bubbleSort$L124
b1$bubbleSort$L126:
	add r15101, r10002, #1
	add r15401, r11001, #1
	add r11201, r10002, #1
	mov r12601, r102
	movw r172, #4
	mul r15201, r15101, r172
	movw r173, #4
	mul r15501, r15401, r173
	add r15301, r12201, r15201
	ldr r15701, [r12401, r15501]
	str r15701, [r15301]
	ldr r11301, [r102]
	movw r186, #0
	cmp r11201, r186
	bge b1$bubbleSort$L128
b1$bubbleSort$L127:
	movw r187, #65535
	movt r187, #65535
	mov r0, r187
	bl exit
b1$bubbleSort$L128:
	cmp r11201, r11301
	bge b1$bubbleSort$L127
b1$bubbleSort$L129:
	add r15901, r11201, #1
	ldr r16301, [r101]
	movw r174, #4
	mul r16001, r15901, r174
	str r16301, [r12601, r16001]
	b b1$bubbleSort$L132
b1$bubbleSort$L102:
	movw r188, #0
	mov r0, r188
	sub sp, fp, #36
	add sp, sp, #4
	pop {r4-r10, fp, lr}
	bx lr

.balign 4
.global main
.section .text
.arm
main:
main$L108:
	push {r4-r10, fp, lr}
	sub sp, sp, #4
	add fp, sp, #36
	movw r144, #32
	mov r0, r144
	bl malloc
	mov r10001, r0
	mov r141, #0
	movw r145, #7
	str r145, [r10001]
	movw r146, #6
	str r146, [r10001, #4]
	movw r147, #3
	str r147, [r10001, #8]
	movw r148, #0
	str r148, [r10001, #12]
	movw r149, #5
	str r149, [r10001, #16]
	movw r150, #9
	str r150, [r10001, #20]
	movw r151, #1
	str r151, [r10001, #24]
	movw r152, #2
	str r152, [r10001, #28]
	movw r153, #8
	mov r0, r153
	bl malloc
	mov r10401, r0
	mov r11101, r10001
	ldr r154, =b1$bubbleSort
	str r154, [r10401, #4]
	mov r10101, r10401
	ldr r11001, [r10101, #4]
	mov r11201, r10101
	ldr r10501, [r10001]
	mov r11301, r11001
	mov r0, r11201
	mov r1, r11101
	mov r2, r10501
	blx r11301
	mov r10202, r141
main$L102:
	mov r11501, r10202
	ldr r10601, [r10001]
	cmp r11501, r10601
	blt main$L103
main$L104:
	movw r157, #10
	mov r0, r157
	bl putch
	movw r158, #0
	mov r0, r158
	sub sp, fp, #36
	add sp, sp, #4
	pop {r4-r10, fp, lr}
	bx lr
main$L103:
	mov r11701, r10001
	ldr r10701, [r10001]
	movw r159, #0
	cmp r10202, r159
	bge main$L106
main$L105:
	movw r160, #65535
	movt r160, #65535
	mov r0, r160
	bl exit
main$L106:
	cmp r10202, r10701
	bge main$L105
main$L107:
	add r13501, r10202, #1
	add r10203, r10202, #1
	movw r155, #4
	mul r13601, r13501, r155
	ldr r11801, [r11701, r13601]
	mov r0, r11801
	bl putint
	movw r156, #32
	mov r0, r156
	bl putch
	mov r10202, r10203
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
