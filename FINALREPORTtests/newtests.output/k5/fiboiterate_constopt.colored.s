.balign 4
.global main
.section .text
.arm
main:
main$L115:
	push {r4-r10, fp, lr}
	sub sp, sp, #16
	add fp, sp, #48
	mov r4, #1
	mov r10, #0
	str r10, [fp, #-44]
	mov r10, #0
	str r10, [fp, #-48]
	ldr r9, [fp, #-48]
	mov r10, r9
	str r10, [fp, #-48]
	ldr r9, [fp, #-44]
	mov r10, r9
	str r10, [fp, #-44]
main$L102:
	movw r0, #47
	ldr r9, [fp, #-48]
	cmp r9, r0
	blt main$L103
main$L104:
	movw r0, #10
	bl putch
	movw r0, #0
	sub sp, fp, #48
	add sp, sp, #16
	pop {r4-r10, fp, lr}
	bx lr
main$L103:
	movw r0, #0
	ldr r9, [fp, #-48]
	cmp r9, r0
	beq main$L112
main$L113:
	movw r0, #1
	ldr r9, [fp, #-48]
	cmp r9, r0
	beq main$L109
main$L110:
	ldr r9, [fp, #-44]
	mov r0, r9
	mov r10, r4
	str r10, [fp, #-40]
	ldr r10, [fp, #-40]
	add r4, r0, r10
	mov r0, r4
	bl putint
	mov r0, r4
	ldr r9, [fp, #-40]
	mov r10, r9
	str r10, [fp, #-44]
main$L111:
	mov r4, r0
	ldr r9, [fp, #-44]
	mov r10, r9
	str r10, [fp, #-44]
main$L114:
	movw r0, #32
	bl putch
	ldr r9, [fp, #-48]
	add r10, r9, #1
	str r10, [fp, #-48]
	ldr r9, [fp, #-48]
	mov r10, r9
	str r10, [fp, #-48]
	ldr r9, [fp, #-44]
	mov r10, r9
	str r10, [fp, #-44]
	b main$L102
main$L109:
	ldr r9, [fp, #-48]
	mov r0, r9
	bl putint
	mov r0, r4
	ldr r9, [fp, #-44]
	mov r10, r9
	str r10, [fp, #-44]
	b main$L111
main$L112:
	ldr r9, [fp, #-48]
	mov r0, r9
	bl putint
	ldr r9, [fp, #-44]
	mov r10, r9
	str r10, [fp, #-44]
	b main$L114

.global malloc
.global getint
.global getch
.global getarray
.global putint
.global putch
.global putarray
.global starttime
.global stoptime
