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
	sub sp, sp, #44
	add fp, sp, #76
	movw r10, #1030
	str r10, [fp, #-60]
	mov r10, #1000
	str r10, [fp, #-44]
	mov r10, #0
	str r10, [fp, #-48]
	mov r4, #0
	mov r10, #0
	str r10, [fp, #-52]
	ldr r9, [fp, #-48]
	mov r10, r9
	str r10, [fp, #-48]
main$L102:
	ldr r9, [fp, #-48]
	ldr r10, [fp, #-60]
	cmp r9, r10
	blt main$L103
main$L104:
	movw r0, #10
	bl putch
	mov r0, r4
	bl putint
	movw r0, #10
	bl putch
	movw r0, #0
	sub sp, fp, #76
	add sp, sp, #44
	pop {r4-r10, fp, lr}
	bx lr
main$L103:
	ldr r9, [fp, #-48]
	add r10, r9, #1
	str r10, [fp, #-48]
	movw r0, #4
	bl malloc
	mov r2, r0
	ldr r9, [fp, #-48]
	ldr r10, [fp, #-44]
	mul r1, r9, r10
	ldr r9, [fp, #-48]
	ldr r10, [fp, #-44]
	mul r0, r9, r10
	movw r3, #10
	ldr r9, [fp, #-48]
	mul r10, r9, r3
	str r10, [fp, #-76]
	ldr r3, =C$a
	str r3, [r2]
	add r10, r4, r0
	str r10, [fp, #-64]
	ldr r3, [r2]
	mov r0, r2
	mov r2, r3
	blx r2
	mov r10, r0
	str r10, [fp, #-40]
	ldr r9, [fp, #-40]
	mov r10, r9
	str r10, [fp, #-40]
	ldr r9, [fp, #-52]
	mov r10, r9
	str r10, [fp, #-56]
main$L107:
	ldr r9, [fp, #-56]
	ldr r10, [fp, #-76]
	cmp r9, r10
	blt main$L108
main$L109:
	ldr r9, [fp, #-56]
	sub r10, r9, #1
	str r10, [fp, #-68]
	ldr r9, [fp, #-40]
	mov r4, r9
	ldr r9, [fp, #-40]
	ldr r10, [r9]
	str r10, [fp, #-72]
	movw r0, #0
	ldr r9, [fp, #-68]
	cmp r9, r0
	bge main$L114
main$L113:
	movw r0, #65535
	movt r0, #65535
	bl exit
main$L114:
	ldr r9, [fp, #-68]
	ldr r10, [fp, #-72]
	cmp r9, r10
	bge main$L113
main$L115:
	ldr r9, [fp, #-68]
	add r0, r9, #1
	movw r1, #4
	mul r0, r0, r1
	ldr r0, [r4, r0]
	bl putint
	movw r0, #32
	bl putch
	ldr r9, [fp, #-64]
	mov r4, r9
	ldr r9, [fp, #-48]
	mov r10, r9
	str r10, [fp, #-48]
	b main$L102
main$L108:
	ldr r9, [fp, #-40]
	ldr r4, [r9]
	movw r0, #0
	ldr r9, [fp, #-56]
	cmp r9, r0
	bge main$L111
main$L110:
	movw r0, #65535
	movt r0, #65535
	bl exit
main$L111:
	ldr r9, [fp, #-56]
	cmp r9, r4
	bge main$L110
main$L112:
	ldr r9, [fp, #-56]
	add r1, r9, #1
	ldr r9, [fp, #-56]
	ldr r10, [fp, #-48]
	mul r0, r9, r10
	ldr r9, [fp, #-56]
	add r2, r9, #1
	movw r3, #4
	mul r1, r1, r3
	ldr r10, [fp, #-40]
	str r0, [r10, r1]
	mov r10, r2
	str r10, [fp, #-56]
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
