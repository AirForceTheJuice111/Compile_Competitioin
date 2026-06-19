.balign 4
.global main
.section .text
.arm
main:
main$L144:
	push {r4-r10, fp, lr}
	sub sp, sp, #4
	add fp, sp, #36
	movw r0, #4
	bl malloc
	mov r6, #1
	ldr r1, =p$p
	str r1, [r0]
	mov r5, r0
main$L102:
	movw r0, #100
	cmp r6, r0
	ble main$L103
main$L104:
	movw r0, #10
	bl putch
main$L145:
	movw r0, #0
	sub sp, fp, #36
	add sp, sp, #4
	pop {r4-r10, fp, lr}
	bx lr
main$L103:
	mov r4, #0
	movw r0, #3
	sdiv r0, r6, r0
	movw r1, #3
	mul r0, r0, r1
	cmp r0, r6
	beq main$L107
main$L120:
main$L121:
	movw r0, #5
	sdiv r0, r6, r0
	movw r1, #5
	mul r0, r0, r1
	cmp r0, r6
	beq main$L124
main$L137:
main$L138:
	movw r0, #0
	cmp r4, r0
	beq main$L141
main$L142:
main$L143:
	movw r0, #32
	bl putch
	add r6, r6, #1
	b main$L102
main$L141:
	mov r0, r6
	bl putint
	b main$L143
main$L124:
	ldr r2, [r5]
	movw r1, #98
	mov r0, r5
	blx r2
	movw r1, #0
	cmp r0, r1
	beq main$L137
main$L127:
	ldr r2, [r5]
	movw r1, #117
	mov r0, r5
	blx r2
	movw r1, #0
	cmp r0, r1
	beq main$L137
main$L130:
	ldr r2, [r5]
	movw r1, #122
	mov r0, r5
	blx r2
	movw r1, #0
	cmp r0, r1
	beq main$L137
main$L133:
	ldr r2, [r5]
	movw r1, #122
	mov r0, r5
	blx r2
	movw r1, #0
	cmp r0, r1
	beq main$L137
main$L136:
	mov r4, #1
	b main$L138
main$L107:
	ldr r2, [r5]
	movw r1, #102
	mov r0, r5
	blx r2
	movw r1, #0
	cmp r0, r1
	beq main$L120
main$L110:
	ldr r2, [r5]
	movw r1, #105
	mov r0, r5
	blx r2
	movw r1, #0
	cmp r0, r1
	beq main$L120
main$L113:
	ldr r2, [r5]
	movw r1, #122
	mov r0, r5
	blx r2
	movw r1, #0
	cmp r0, r1
	beq main$L120
main$L116:
	ldr r2, [r5]
	movw r1, #122
	mov r0, r5
	blx r2
	movw r1, #0
	cmp r0, r1
	beq main$L120
main$L119:
	mov r4, #1
	b main$L121

.balign 4
.global p$p
.section .text
.arm
p$p:
p$p$L100:
	push {r4-r10, fp, lr}
	sub sp, sp, #4
	add fp, sp, #36
	mov r0, r1
	bl putch
	movw r0, #1
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
