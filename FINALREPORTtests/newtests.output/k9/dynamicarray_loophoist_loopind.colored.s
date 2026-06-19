.balign 4
.global main
.section .text
.arm
main:
main$L116:
	push {r4-r10, fp, lr}
	sub sp, sp, #40
	add fp, sp, #72
	mov r10, #1000
	str r10, [fp, #-40]
	mov r5, #0
	mov r6, #0
	mov r10, #0
	str r10, [fp, #-48]
	movw r10, #1030
	str r10, [fp, #-52]
	add r3, r5, #1
	add r2, r5, #1
	add r0, r5, #1
	add r1, r5, #1
	ldr r10, [fp, #-48]
	mul r1, r1, r10
	ldr r10, [fp, #-40]
	mul r4, r3, r10
	ldr r10, [fp, #-40]
	mul r3, r2, r10
	movw r2, #10
	mul r2, r0, r2
	mov r7, r6
	mov r0, r5
	mov r10, r4
	str r10, [fp, #-60]
	mov r10, r3
	str r10, [fp, #-64]
	mov r10, r2
	str r10, [fp, #-68]
	mov r10, r1
	str r10, [fp, #-72]
main$L102:
	ldr r10, [fp, #-52]
	cmp r0, r10
	blt main$L103
main$L104:
	movw r0, #10
	bl putch
	mov r0, r7
	bl putint
	movw r0, #10
	bl putch
	movw r0, #0
	sub sp, fp, #72
	add sp, sp, #40
	pop {r4-r10, fp, lr}
	bx lr
main$L103:
	add r10, r0, #1
	str r10, [fp, #-44]
	movw r0, #4
	bl malloc
	ldr r10, [fp, #-64]
	add r10, r7, r10
	str r10, [fp, #-56]
	movw r1, #4
	ldr r9, [fp, #-48]
	mul r1, r9, r1
	ldr r2, =C$a
	str r2, [r0]
	add r4, r1, #4
	ldr r1, [r0]
	mov r2, r1
	ldr r9, [fp, #-60]
	mov r1, r9
	blx r2
	mov r8, r0
	ldr r9, [fp, #-48]
	mov r7, r9
	mov r5, r4
	ldr r9, [fp, #-72]
	mov r4, r9
main$L107:
	ldr r10, [fp, #-68]
	cmp r7, r10
	blt main$L108
main$L109:
	sub r4, r7, #1
	mov r7, r8
	ldr r5, [r8]
	movw r0, #0
	cmp r4, r0
	bge main$L114
main$L113:
	movw r0, #65535
	movt r0, #65535
	bl exit
main$L114:
	cmp r4, r5
	bge main$L113
main$L115:
	add r0, r4, #1
	ldr r9, [fp, #-60]
	ldr r10, [fp, #-40]
	add r8, r9, r10
	ldr r9, [fp, #-64]
	ldr r10, [fp, #-40]
	add r6, r9, r10
	ldr r9, [fp, #-68]
	add r5, r9, #10
	ldr r9, [fp, #-72]
	ldr r10, [fp, #-48]
	add r4, r9, r10
	movw r1, #4
	mul r0, r0, r1
	ldr r0, [r7, r0]
	bl putint
	movw r0, #32
	bl putch
	ldr r9, [fp, #-56]
	mov r7, r9
	ldr r9, [fp, #-44]
	mov r0, r9
	mov r10, r8
	str r10, [fp, #-60]
	mov r10, r6
	str r10, [fp, #-64]
	mov r10, r5
	str r10, [fp, #-68]
	mov r10, r4
	str r10, [fp, #-72]
	b main$L102
main$L108:
	ldr r6, [r8]
	movw r0, #0
	cmp r7, r0
	bge main$L111
main$L110:
	movw r0, #65535
	movt r0, #65535
	bl exit
main$L111:
	cmp r7, r6
	bge main$L110
main$L112:
	str r4, [r8, r5]
	add r7, r7, #1
	add r5, r5, #4
	ldr r10, [fp, #-44]
	add r4, r4, r10
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
