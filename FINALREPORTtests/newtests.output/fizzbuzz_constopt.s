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
	movw r107, #1
	mov r0, r107
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
	movw r156, #4
	mov r0, r156
	bl malloc
	mov r10401, r0
	mov r150, #1
	ldr r157, =p$p
	str r157, [r10401]
	mov r10101, r10401
	mov r10002, r150
main$L102:
	movw r172, #100
	cmp r10002, r172
	ble main$L103
main$L104:
	movw r171, #10
	mov r0, r171
	bl putch
main$L145:
	movw r173, #0
	mov r0, r173
	sub sp, fp, #36
	add sp, sp, #4
	pop {r4-r10, fp, lr}
	bx lr
main$L103:
	movw r158, #3
	sdiv r12701, r10002, r158
	movw r159, #3
	mul r12801, r12701, r159
	cmp r12801, r10002
	beq main$L107
main$L120:
	mov r152, #0
	mov r10203, r152
main$L121:
	movw r164, #5
	sdiv r13701, r10002, r164
	movw r165, #5
	mul r13801, r13701, r165
	cmp r13801, r10002
	beq main$L124
main$L137:
	mov r10205, r10203
main$L138:
	movw r174, #0
	cmp r10205, r174
	beq main$L141
main$L142:
main$L143:
	movw r170, #32
	mov r0, r170
	bl putch
	add r10003, r10002, #1
	mov r10002, r10003
	b main$L102
main$L141:
	mov r0, r10002
	bl putint
	b main$L143
main$L124:
	ldr r11401, [r10101]
	movw r166, #98
	mov r0, r10101
	mov r1, r166
	blx r11401
	mov r11501, r0
	movw r175, #0
	cmp r11501, r175
	beq main$L137
main$L127:
	ldr r11601, [r10101]
	movw r167, #117
	mov r0, r10101
	mov r1, r167
	blx r11601
	mov r11701, r0
	movw r176, #0
	cmp r11701, r176
	beq main$L137
main$L130:
	ldr r11801, [r10101]
	movw r168, #122
	mov r0, r10101
	mov r1, r168
	blx r11801
	mov r11901, r0
	movw r177, #0
	cmp r11901, r177
	beq main$L137
main$L133:
	ldr r12001, [r10101]
	movw r169, #122
	mov r0, r10101
	mov r1, r169
	blx r12001
	mov r12101, r0
	movw r178, #0
	cmp r12101, r178
	beq main$L137
main$L136:
	mov r153, #1
	mov r10205, r153
	b main$L138
main$L107:
	ldr r10601, [r10101]
	movw r160, #102
	mov r0, r10101
	mov r1, r160
	blx r10601
	mov r10701, r0
	movw r179, #0
	cmp r10701, r179
	beq main$L120
main$L110:
	ldr r10801, [r10101]
	movw r161, #105
	mov r0, r10101
	mov r1, r161
	blx r10801
	mov r10901, r0
	movw r180, #0
	cmp r10901, r180
	beq main$L120
main$L113:
	ldr r11001, [r10101]
	movw r162, #122
	mov r0, r10101
	mov r1, r162
	blx r11001
	mov r11101, r0
	movw r181, #0
	cmp r11101, r181
	beq main$L120
main$L116:
	ldr r11201, [r10101]
	movw r163, #122
	mov r0, r10101
	mov r1, r163
	blx r11201
	mov r11301, r0
	movw r182, #0
	cmp r11301, r182
	beq main$L120
main$L119:
	mov r151, #1
	mov r10203, r151
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
