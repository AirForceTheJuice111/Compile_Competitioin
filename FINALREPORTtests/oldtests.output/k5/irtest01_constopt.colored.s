.balign 4
.global main
.section .text
.arm
main:
main$L113:
	push {r4-r10, fp, lr}
	sub sp, sp, #8
	add fp, sp, #40
	bl getint
	mov r4, r0
	movw r0, #0
	cmp r4, r0
	blt main$L105
main$L104:
	movw r0, #47
	cmp r4, r0
	bgt main$L105
main$L106:
main$L107:
	mov r10, #0
	str r10, [fp, #-40]
	ldr r9, [fp, #-40]
	mov r10, r9
	str r10, [fp, #-40]
main$L110:
	ldr r9, [fp, #-40]
	cmp r9, r4
	blt main$L111
main$L112:
	movw r0, #0
	sub sp, fp, #40
	add sp, sp, #8
	pop {r4-r10, fp, lr}
	bx lr
main$L111:
	movw r0, #32
	bl putch
	ldr r9, [fp, #-40]
	add r10, r9, #1
	str r10, [fp, #-40]
	ldr r9, [fp, #-40]
	mov r10, r9
	str r10, [fp, #-40]
	b main$L110
main$L105:
	movw r0, #65535
	movt r0, #65535
	sub sp, fp, #40
	add sp, sp, #8
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
