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

.balign 4
.global main
.section .text
.arm
main:
main$L116:
	push {r4-r10, fp, lr}
	sub sp, sp, #12
	add fp, sp, #44
	movw r8, #1030
	mov r10, #1000
	str r10, [fp, #-40]
	mov r10, #0
	str r10, [fp, #-44]
	mov r5, #0
	ldr r9, [fp, #-44]
	mov r10, r9
	str r10, [fp, #-44]
main$L102:
	ldr r9, [fp, #-44]
	cmp r9, r8
	blt main$L103
main$L104:
	movw r0, #10
	bl putch
	mov r0, r5
	bl putint
	movw r0, #10
	bl putch
	movw r0, #0
	sub sp, fp, #44
	add sp, sp, #12
	pop {r4-r10, fp, lr}
	bx lr
main$L103:
	ldr r9, [fp, #-44]
	add r10, r9, #1
	str r10, [fp, #-44]
	movw r0, #4
	bl malloc
	mov r2, r0
	mov r4, #0
	ldr r9, [fp, #-44]
	ldr r10, [fp, #-40]
	mul r1, r9, r10
	ldr r9, [fp, #-44]
	ldr r10, [fp, #-40]
	mul r0, r9, r10
	ldr r3, =C$a
	str r3, [r2]
	add r6, r5, r0
	ldr r3, [r2]
	mov r0, r2
	mov r2, r3
	blx r2
	mov r5, r0
	mov r7, r4
main$L107:
	movw r0, #10
	ldr r9, [fp, #-44]
	mul r0, r9, r0
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
	mov r5, r6
	ldr r9, [fp, #-44]
	mov r10, r9
	str r10, [fp, #-44]
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
	ldr r10, [fp, #-44]
	mul r0, r7, r10
	add r2, r7, #1
	movw r3, #4
	mul r1, r1, r3
	str r0, [r5, r1]
	mov r7, r2
	b main$L107

.global malloc
.global getint
.global getch
.global getarray
.global putint
.global putch
.global putarray
.global starttime
.global stoptime
