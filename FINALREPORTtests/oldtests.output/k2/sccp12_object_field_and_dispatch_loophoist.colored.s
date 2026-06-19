.balign 4
.global main
.section .text
.arm
main:
main$L105:
	push {r4-r10, fp, lr}
	sub sp, sp, #16
	add fp, sp, #48
	movw r0, #8
	bl malloc
	ldr r1, =Child$value
	str r1, [r0, #4]
	mov r10, r0
	str r10, [fp, #-44]
	ldr r9, [fp, #-44]
	ldr r10, [r9, #4]
	str r10, [fp, #-48]
	movw r1, #10
	ldr r9, [fp, #-44]
	mov r0, r9
	ldr r9, [fp, #-48]
	blx r9
	movw r10, #1
	str r10, [fp, #-40]
	movw r1, #0
	ldr r9, [fp, #-40]
	cmp r9, r1
	bne main$L102
main$L103:
	movw r0, #0
	ldr r10, [fp, #-44]
	str r0, [r10]
main$L104:
	ldr r9, [fp, #-44]
	ldr r0, [r9]
	sub sp, fp, #48
	add sp, sp, #16
	pop {r4-r10, fp, lr}
	bx lr
main$L102:
	ldr r10, [fp, #-44]
	str r0, [r10]
	b main$L104

.balign 4
.global Base$value
.section .text
.arm
Base$value:
Base$value$L100:
	push {r4-r10, fp, lr}
	sub sp, sp, #4
	add fp, sp, #36
	add r0, r1, #1
	sub sp, fp, #36
	add sp, sp, #4
	pop {r4-r10, fp, lr}
	bx lr

.balign 4
.global Child$value
.section .text
.arm
Child$value:
Child$value$L100:
	push {r4-r10, fp, lr}
	sub sp, sp, #4
	add fp, sp, #36
	add r0, r1, #2
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
