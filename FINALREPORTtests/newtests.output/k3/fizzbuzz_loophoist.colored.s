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

.balign 4
.global main
.section .text
.arm
main:
main$L144:
	push {r4-r10, fp, lr}
	sub sp, sp, #56
	add fp, sp, #88
	movw r0, #4
	bl malloc
	mov r1, #1
	mov r10, #0
	str r10, [fp, #-44]
	mov r10, #1
	str r10, [fp, #-48]
	mov r10, #1
	str r10, [fp, #-56]
	ldr r2, =p$p
	str r2, [r0]
	mov r10, r0
	str r10, [fp, #-60]
	ldr r9, [fp, #-60]
	mov r10, r9
	str r10, [fp, #-60]
	ldr r9, [fp, #-60]
	mov r10, r9
	str r10, [fp, #-64]
	ldr r9, [fp, #-60]
	mov r10, r9
	str r10, [fp, #-68]
	ldr r9, [fp, #-60]
	mov r10, r9
	str r10, [fp, #-72]
	ldr r9, [fp, #-60]
	mov r10, r9
	str r10, [fp, #-76]
	ldr r9, [fp, #-60]
	mov r10, r9
	str r10, [fp, #-80]
	ldr r9, [fp, #-60]
	mov r10, r9
	str r10, [fp, #-84]
	ldr r9, [fp, #-60]
	mov r10, r9
	str r10, [fp, #-88]
	mov r10, r1
	str r10, [fp, #-40]
main$L102:
	movw r0, #100
	ldr r9, [fp, #-40]
	cmp r9, r0
	ble main$L103
main$L104:
	movw r0, #10
	bl putch
main$L145:
	movw r0, #0
	sub sp, fp, #88
	add sp, sp, #56
	pop {r4-r10, fp, lr}
	bx lr
main$L103:
	movw r0, #3
	ldr r9, [fp, #-40]
	sdiv r0, r9, r0
	movw r1, #3
	mul r0, r0, r1
	ldr r10, [fp, #-40]
	cmp r0, r10
	beq main$L107
main$L120:
	ldr r9, [fp, #-44]
	mov r10, r9
	str r10, [fp, #-52]
main$L121:
	movw r0, #5
	ldr r9, [fp, #-40]
	sdiv r0, r9, r0
	movw r1, #5
	mul r0, r0, r1
	ldr r10, [fp, #-40]
	cmp r0, r10
	beq main$L124
main$L137:
	ldr r9, [fp, #-52]
	mov r0, r9
main$L138:
	movw r1, #0
	cmp r0, r1
	beq main$L141
main$L142:
main$L143:
	movw r0, #32
	bl putch
	ldr r9, [fp, #-40]
	add r10, r9, #1
	str r10, [fp, #-40]
	ldr r9, [fp, #-40]
	mov r10, r9
	str r10, [fp, #-40]
	b main$L102
main$L141:
	ldr r9, [fp, #-40]
	mov r0, r9
	bl putint
	b main$L143
main$L124:
	ldr r9, [fp, #-76]
	ldr r2, [r9]
	movw r1, #98
	ldr r9, [fp, #-60]
	mov r0, r9
	blx r2
	movw r1, #0
	cmp r0, r1
	beq main$L137
main$L127:
	ldr r9, [fp, #-80]
	ldr r2, [r9]
	movw r1, #117
	ldr r9, [fp, #-60]
	mov r0, r9
	blx r2
	movw r1, #0
	cmp r0, r1
	beq main$L137
main$L130:
	ldr r9, [fp, #-84]
	ldr r2, [r9]
	movw r1, #122
	ldr r9, [fp, #-60]
	mov r0, r9
	blx r2
	movw r1, #0
	cmp r0, r1
	beq main$L137
main$L133:
	ldr r9, [fp, #-88]
	ldr r2, [r9]
	movw r1, #122
	ldr r9, [fp, #-60]
	mov r0, r9
	blx r2
	movw r1, #0
	cmp r0, r1
	beq main$L137
main$L136:
	ldr r9, [fp, #-56]
	mov r0, r9
	b main$L138
main$L107:
	ldr r9, [fp, #-60]
	ldr r2, [r9]
	movw r1, #102
	ldr r9, [fp, #-60]
	mov r0, r9
	blx r2
	movw r1, #0
	cmp r0, r1
	beq main$L120
main$L110:
	ldr r9, [fp, #-64]
	ldr r2, [r9]
	movw r1, #105
	ldr r9, [fp, #-60]
	mov r0, r9
	blx r2
	movw r1, #0
	cmp r0, r1
	beq main$L120
main$L113:
	ldr r9, [fp, #-68]
	ldr r2, [r9]
	movw r1, #122
	ldr r9, [fp, #-60]
	mov r0, r9
	blx r2
	movw r1, #0
	cmp r0, r1
	beq main$L120
main$L116:
	ldr r9, [fp, #-72]
	ldr r2, [r9]
	movw r1, #122
	ldr r9, [fp, #-60]
	mov r0, r9
	blx r2
	movw r1, #0
	cmp r0, r1
	beq main$L120
main$L119:
	ldr r9, [fp, #-48]
	mov r10, r9
	str r10, [fp, #-52]
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
