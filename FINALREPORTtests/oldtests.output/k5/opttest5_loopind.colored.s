.balign 4
.global main
.section .text
.arm
main:
main$L115:
	push {r4-r10, fp, lr}
	sub sp, sp, #16
	add fp, sp, #48
	bl getint
	mov r2, r0
	mov r0, #10
	mov r4, #0
	add r1, r4, #1
	movw r3, #3
	mul r1, r1, r3
	mov r10, r0
	str r10, [fp, #-48]
	ldr r9, [fp, #-40]
	mov r10, r9
	str r10, [fp, #-44]
	mov r3, r4
	mov r0, r1
main$L102:
	movw r1, #0
	ldr r9, [fp, #-48]
	cmp r9, r1
	bgt main$L103
main$L104:
	ldr r9, [fp, #-44]
	mov r0, r9
	sub sp, fp, #48
	add sp, sp, #16
	pop {r4-r10, fp, lr}
	bx lr
main$L103:
	add r3, r3, #1
	ldr r9, [fp, #-44]
	mov r1, r9
main$L107:
	movw r4, #0
	cmp r2, r4
	bgt main$L108
main$L109:
	ldr r9, [fp, #-48]
	sub r10, r9, #1
	str r10, [fp, #-48]
	add r0, r0, #3
	ldr r9, [fp, #-48]
	mov r10, r9
	str r10, [fp, #-48]
	mov r10, r1
	str r10, [fp, #-44]
	b main$L102
main$L108:
	movw r1, #3
	cmp r3, r1
	bgt main$L112
main$L113:
	mov r1, r2
main$L114:
	sub r2, r2, #1
	b main$L107
main$L112:
	mov r1, r0
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
