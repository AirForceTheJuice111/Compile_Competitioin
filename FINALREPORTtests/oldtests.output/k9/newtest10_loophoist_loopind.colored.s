.balign 4
.global main
.section .text
.arm
main:
main$L116:
	push {r4-r10, fp, lr}
	sub sp, sp, #4
	add fp, sp, #36
	mov r8, #0
	mov r6, #100
	add r0, r8, #1
	mul r4, r8, r8
	add r1, r6, #1
	movw r2, #4
	mul r5, r0, r2
	movw r0, #4
	mul r0, r1, r0
	bl malloc
	str r6, [r0]
	mov r6, r0
	add r5, r6, r5
main$L102:
	ldr r0, [r6]
	cmp r8, r0
	blt main$L103
main$L104:
	ldr r0, [r6]
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
	ldr r4, [r6]
	movw r1, #99
	movw r0, #0
	cmp r1, r0
	bge main$L111
main$L110:
	movw r0, #65535
	movt r0, #65535
	bl exit
main$L111:
	movw r0, #99
	cmp r0, r4
	bge main$L110
main$L112:
	movw r1, #99
	movw r0, #1
	add r0, r1, r0
	movw r1, #4
	mul r0, r0, r1
	ldr r0, [r6, r0]
	bl putint
	b main$L115
main$L103:
	ldr r7, [r6]
	movw r0, #0
	cmp r8, r0
	bge main$L106
main$L105:
	movw r0, #65535
	movt r0, #65535
	bl exit
main$L106:
	cmp r8, r7
	bge main$L105
main$L107:
	str r4, [r5]
	b main$L102

.global malloc
.global getint
.global getch
.global getarray
.global putint
.global putch
.global putarray
.global starttime
.global stoptime
