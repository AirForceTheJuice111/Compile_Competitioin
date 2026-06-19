.balign 4
.global main
.section .text
.arm
main:
main$L110:
	push {r4-r10, fp, lr}
	sub sp, sp, #16
	add fp, sp, #48
	bl getint
	mov r10, r0
	str r10, [fp, #-40]
	mov r0, #10
	mov r1, #0
	mov r10, #4
	str r10, [fp, #-48]
	movw r2, #3
	mul r0, r0, r2
	add r0, r0, #2
	mov r10, r1
	str r10, [fp, #-44]
	mov r4, r0
main$L102:
	movw r0, #2
	cmp r4, r0
	bgt main$L103
main$L104:
	ldr r9, [fp, #-44]
	mov r0, r9
	bl putint
	movw r0, #10
	bl putch
	ldr r9, [fp, #-44]
	mov r0, r9
	sub sp, fp, #48
	add sp, sp, #16
	pop {r4-r10, fp, lr}
	bx lr
main$L103:
	movw r0, #0
	ldr r9, [fp, #-40]
	cmp r9, r0
	bgt main$L107
main$L108:
	ldr r9, [fp, #-48]
	mov r10, r9
	str r10, [fp, #-44]
main$L109:
	sub r4, r4, #3
	ldr r9, [fp, #-44]
	mov r10, r9
	str r10, [fp, #-44]
	b main$L102
main$L107:
	mov r0, r4
	bl putint
	movw r0, #32
	bl putch
	mov r10, r4
	str r10, [fp, #-44]
	b main$L109

.global malloc
.global getint
.global getch
.global getarray
.global putint
.global putch
.global putarray
.global starttime
.global stoptime
