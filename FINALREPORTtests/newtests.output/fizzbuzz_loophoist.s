.balign 4
.global p$p
.section .text
.arm
p$p:
p$p$L100:
	push {r4-r10, fp, lr}
	sub sp, sp, #4
	add fp, sp, #36
	mov r100, r0
	mov r101, r1
	mov r0, r101
	bl putch
	movw r105, #1
	mov r0, r105
	sub sp, fp, #36
	add sp, sp, #4
	pop {r4-r10, fp, lr}
	bx lr

.balign 4
.global main
.section .text
.arm
main:
main$L144:
	push {r4-r10, fp, lr}
	sub sp, sp, #4
	add fp, sp, #36
	movw r150, #4
	mov r0, r150
	bl malloc
	mov r10401, r0
	mov r10001, #1
	mov r10201, #0
	mov r10202, #1
	mov r10204, #1
	ldr r151, =p$p
	str r151, [r10401]
	mov r10101, r10401
	mov r12901, r10101
	mov r13101, r10101
	mov r13301, r10101
	mov r13501, r10101
	mov r13901, r10101
	mov r14101, r10101
	mov r14301, r10101
	mov r14501, r10101
	mov r10002, r10001
main$L102:
	movw r166, #100
	cmp r10002, r166
	ble main$L103
main$L104:
	movw r165, #10
	mov r0, r165
	bl putch
main$L145:
	movw r167, #0
	mov r0, r167
	sub sp, fp, #36
	add sp, sp, #4
	pop {r4-r10, fp, lr}
	bx lr
main$L103:
	movw r152, #3
	sdiv r12701, r10002, r152
	movw r153, #3
	mul r12801, r12701, r153
	cmp r12801, r10002
	beq main$L107
main$L120:
	mov r10203, r10201
main$L121:
	movw r158, #5
	sdiv r13701, r10002, r158
	movw r159, #5
	mul r13801, r13701, r159
	cmp r13801, r10002
	beq main$L124
main$L137:
	mov r10205, r10203
main$L138:
	movw r168, #0
	cmp r10205, r168
	beq main$L141
main$L142:
main$L143:
	movw r164, #32
	mov r0, r164
	bl putch
	add r10003, r10002, #1
	mov r10002, r10003
	b main$L102
main$L141:
	mov r0, r10002
	bl putint
	b main$L143
main$L124:
	ldr r11401, [r13901]
	movw r160, #98
	mov r0, r10101
	mov r1, r160
	blx r11401
	mov r11501, r0
	movw r169, #0
	cmp r11501, r169
	beq main$L137
main$L127:
	ldr r11601, [r14101]
	movw r161, #117
	mov r0, r10101
	mov r1, r161
	blx r11601
	mov r11701, r0
	movw r170, #0
	cmp r11701, r170
	beq main$L137
main$L130:
	ldr r11801, [r14301]
	movw r162, #122
	mov r0, r10101
	mov r1, r162
	blx r11801
	mov r11901, r0
	movw r171, #0
	cmp r11901, r171
	beq main$L137
main$L133:
	ldr r12001, [r14501]
	movw r163, #122
	mov r0, r10101
	mov r1, r163
	blx r12001
	mov r12101, r0
	movw r172, #0
	cmp r12101, r172
	beq main$L137
main$L136:
	mov r10205, r10204
	b main$L138
main$L107:
	ldr r10601, [r12901]
	movw r154, #102
	mov r0, r10101
	mov r1, r154
	blx r10601
	mov r10701, r0
	movw r173, #0
	cmp r10701, r173
	beq main$L120
main$L110:
	ldr r10801, [r13101]
	movw r155, #105
	mov r0, r10101
	mov r1, r155
	blx r10801
	mov r10901, r0
	movw r174, #0
	cmp r10901, r174
	beq main$L120
main$L113:
	ldr r11001, [r13301]
	movw r156, #122
	mov r0, r10101
	mov r1, r156
	blx r11001
	mov r11101, r0
	movw r175, #0
	cmp r11101, r175
	beq main$L120
main$L116:
	ldr r11201, [r13501]
	movw r157, #122
	mov r0, r10101
	mov r1, r157
	blx r11201
	mov r11301, r0
	movw r176, #0
	cmp r11301, r176
	beq main$L120
main$L119:
	mov r10203, r10202
	b main$L121

.global malloc
.global getint
.global getch
.global getarray
.global putint
.global putch
.global putarray
.global starttime
.global stoptime
