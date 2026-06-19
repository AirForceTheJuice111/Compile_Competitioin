.balign 4
.global main
.section .text
.arm
main:
main$L116:
	push {r4-r10, fp, lr}
	sub sp, sp, #4
	add fp, sp, #36
	movw r0, #404
	bl malloc
	movw r1, #100
	str r1, [r0]
	mov r4, r0
main$L102:
	ldr r0, [r4]
	movw r1, #0
	cmp r1, r0
	blt main$L103
main$L104:
	ldr r0, [r4]
	movw r1, #99
	cmp r0, r1
	bgt main$L113
main$L114:
main$L115:
	movw r0, #10
	bl putch
	movw r0, #1
	sub sp, fp, #36
	add sp, sp, #4
	pop {r4-r10, fp, lr}
	bx lr
main$L113:
	ldr r0, [r4]
main$L111:
	movw r1, #99
	cmp r1, r0
	bge main$L110
main$L112:
	ldr r0, [r4, #400]
	bl putint
	b main$L115
main$L110:
	movw r0, #65535
	movt r0, #65535
	bl exit
main$L103:
	ldr r0, [r4]
main$L106:
	movw r1, #0
	cmp r1, r0
	bge main$L105
main$L107:
	movw r0, #0
	str r0, [r4, #4]
	b main$L102
main$L105:
	movw r0, #65535
	movt r0, #65535
	bl exit

.global malloc
.global getint
.global getch
.global getarray
.global putint
.global putch
.global putarray
.global starttime
.global stoptime
