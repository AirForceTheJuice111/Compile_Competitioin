.balign 4
.global main
.section .text
.arm
main:
main$L116:
	push {r4-r10, fp, lr}
	sub sp, sp, #4
	add fp, sp, #36
	mov r6, #0
	mov r8, #0
main$L102:
	movw r0, #1030
	cmp r8, r0
	blt main$L103
main$L104:
	movw r0, #10
	bl putch
	mov r0, r6
	bl putint
	movw r0, #10
	bl putch
	movw r0, #0
	sub sp, fp, #36
	add sp, sp, #4
	pop {r4-r10, fp, lr}
	bx lr
main$L103:
	add r8, r8, #1
	movw r0, #4
	bl malloc
	mov r4, #0
	movw r1, #1000
	mul r1, r8, r1
	movw r2, #1000
	mul r2, r8, r2
	ldr r3, =C$a
	str r3, [r0]
	add r6, r6, r2
	ldr r2, [r0]
	blx r2
	mov r5, r0
	mov r7, r4
main$L107:
	movw r0, #10
	mul r0, r8, r0
	cmp r7, r0
	blt main$L108
main$L109:
	sub r7, r7, #1
	mov r4, r5
	ldr r5, [r5]
	movw r0, #0
	cmp r7, r0
	bge main$L114
main$L113:
	movw r0, #65535
	movt r0, #65535
	bl exit
main$L114:
	cmp r7, r5
	bge main$L113
main$L115:
	add r0, r7, #1
	movw r1, #4
	mul r0, r0, r1
	ldr r0, [r4, r0]
	bl putint
	movw r0, #32
	bl putch
	b main$L102
main$L108:
	ldr r4, [r5]
	movw r0, #0
	cmp r7, r0
	bge main$L111
main$L110:
	movw r0, #65535
	movt r0, #65535
	bl exit
main$L111:
	cmp r7, r4
	bge main$L110
main$L112:
	add r1, r7, #1
	mul r0, r7, r8
	add r7, r7, #1
	movw r2, #4
	mul r1, r1, r2
	str r0, [r5, r1]
	b main$L107

.balign 4
.global C$a
.section .text
.arm
C$a:
C$a$L100:
	push {r4-r10, fp, lr}
	sub sp, sp, #4
	add fp, sp, #36
	mov r4, r1
	add r0, r4, #1
	movw r1, #4
	mul r0, r0, r1
	bl malloc
	str r4, [r0]
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
