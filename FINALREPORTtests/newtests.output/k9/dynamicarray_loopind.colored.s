.balign 4
.global main
.section .text
.arm
main:
main$L116:
	push {r4-r10, fp, lr}
	sub sp, sp, #32
	add fp, sp, #64
	movw r10, #1030
	str r10, [fp, #-48]
	mov r10, #1000
	str r10, [fp, #-40]
	mov r10, #0
	str r10, [fp, #-44]
	mov r10, #0
	str r10, [fp, #-52]
	ldr r9, [fp, #-44]
	add r2, r9, #1
	ldr r9, [fp, #-44]
	add r1, r9, #1
	ldr r9, [fp, #-44]
	add r0, r9, #1
	ldr r10, [fp, #-40]
	mul r10, r2, r10
	str r10, [fp, #-56]
	ldr r10, [fp, #-40]
	mul r10, r1, r10
	str r10, [fp, #-60]
	movw r1, #10
	mul r10, r0, r1
	str r10, [fp, #-64]
	ldr r9, [fp, #-52]
	mov r10, r9
	str r10, [fp, #-52]
	ldr r9, [fp, #-44]
	mov r10, r9
	str r10, [fp, #-44]
	ldr r9, [fp, #-56]
	mov r10, r9
	str r10, [fp, #-56]
	ldr r9, [fp, #-60]
	mov r10, r9
	str r10, [fp, #-60]
	ldr r9, [fp, #-64]
	mov r10, r9
	str r10, [fp, #-64]
main$L102:
	ldr r9, [fp, #-44]
	ldr r10, [fp, #-48]
	cmp r9, r10
	blt main$L103
main$L104:
	movw r0, #10
	bl putch
	ldr r9, [fp, #-52]
	mov r0, r9
	bl putint
	movw r0, #10
	bl putch
	movw r0, #0
	sub sp, fp, #64
	add sp, sp, #32
	pop {r4-r10, fp, lr}
	bx lr
main$L103:
	ldr r9, [fp, #-44]
	add r10, r9, #1
	str r10, [fp, #-44]
	movw r0, #4
	bl malloc
	ldr r9, [fp, #-52]
	ldr r10, [fp, #-60]
	add r10, r9, r10
	str r10, [fp, #-52]
	mov r6, #0
	ldr r10, [fp, #-44]
	mul r4, r6, r10
	ldr r1, =C$a
	str r1, [r0]
	movw r1, #4
	mul r2, r6, r1
	ldr r1, [r0]
	add r5, r2, #4
	mov r2, r1
	ldr r9, [fp, #-56]
	mov r1, r9
	blx r2
	mov r8, r0
main$L107:
	ldr r10, [fp, #-64]
	cmp r6, r10
	blt main$L108
main$L109:
	sub r4, r6, #1
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
	ldr r9, [fp, #-56]
	ldr r10, [fp, #-40]
	add r6, r9, r10
	ldr r9, [fp, #-60]
	ldr r10, [fp, #-40]
	add r5, r9, r10
	ldr r9, [fp, #-64]
	add r4, r9, #10
	movw r1, #4
	mul r0, r0, r1
	ldr r0, [r7, r0]
	bl putint
	movw r0, #32
	bl putch
	ldr r9, [fp, #-52]
	mov r10, r9
	str r10, [fp, #-52]
	ldr r9, [fp, #-44]
	mov r10, r9
	str r10, [fp, #-44]
	mov r10, r6
	str r10, [fp, #-56]
	mov r10, r5
	str r10, [fp, #-60]
	mov r10, r4
	str r10, [fp, #-64]
	b main$L102
main$L108:
	ldr r7, [r8]
	movw r0, #0
	cmp r6, r0
	bge main$L111
main$L110:
	movw r0, #65535
	movt r0, #65535
	bl exit
main$L111:
	cmp r6, r7
	bge main$L110
main$L112:
	str r4, [r8, r5]
	add r6, r6, #1
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
