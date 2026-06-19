.balign 4
.global main
.section .text
.arm
main:
main$L116:
	push {r4-r10, fp, lr}
	sub sp, sp, #116
	add fp, sp, #148
	mov r10, #1000
	str r10, [fp, #-44]
	mov r10, #0
	str r10, [fp, #-48]
	mov r10, #0
	str r10, [fp, #-68]
	mov r10, #0
	str r10, [fp, #-56]
	movw r10, #1030
	str r10, [fp, #-64]
	ldr r9, [fp, #-48]
	add r2, r9, #1
	ldr r9, [fp, #-48]
	add r1, r9, #1
	ldr r9, [fp, #-48]
	add r10, r9, #1
	str r10, [fp, #-124]
	ldr r9, [fp, #-48]
	add r0, r9, #1
	ldr r10, [fp, #-56]
	mul r10, r0, r10
	str r10, [fp, #-144]
	ldr r10, [fp, #-44]
	mul r10, r2, r10
	str r10, [fp, #-96]
	ldr r10, [fp, #-44]
	mul r1, r1, r10
	movw r0, #10
	ldr r9, [fp, #-124]
	mul r0, r9, r0
	ldr r9, [fp, #-68]
	mov r10, r9
	str r10, [fp, #-72]
	ldr r9, [fp, #-48]
	mov r2, r9
	ldr r9, [fp, #-96]
	mov r10, r9
	str r10, [fp, #-92]
	mov r10, r1
	str r10, [fp, #-108]
	mov r10, r0
	str r10, [fp, #-116]
	ldr r9, [fp, #-144]
	mov r10, r9
	str r10, [fp, #-140]
main$L102:
	ldr r10, [fp, #-64]
	cmp r2, r10
	blt main$L103
main$L104:
	movw r0, #10
	bl putch
	ldr r9, [fp, #-72]
	mov r0, r9
	bl putint
	movw r0, #10
	bl putch
	movw r0, #0
	sub sp, fp, #148
	add sp, sp, #116
	pop {r4-r10, fp, lr}
	bx lr
main$L103:
	add r10, r2, #1
	str r10, [fp, #-52]
	movw r0, #4
	bl malloc
	mov r1, r0
	ldr r9, [fp, #-72]
	ldr r10, [fp, #-108]
	add r10, r9, r10
	str r10, [fp, #-76]
	movw r0, #4
	ldr r9, [fp, #-56]
	mul r0, r9, r0
	ldr r2, =C$a
	str r2, [r1]
	add r10, r0, #4
	str r10, [fp, #-132]
	ldr r0, [r1]
	mov r2, r0
	mov r0, r1
	ldr r9, [fp, #-92]
	mov r1, r9
	blx r2
	mov r10, r0
	str r10, [fp, #-40]
	ldr r9, [fp, #-40]
	mov r10, r9
	str r10, [fp, #-40]
	ldr r9, [fp, #-56]
	mov r10, r9
	str r10, [fp, #-60]
	ldr r9, [fp, #-132]
	mov r10, r9
	str r10, [fp, #-128]
	ldr r9, [fp, #-140]
	mov r10, r9
	str r10, [fp, #-136]
main$L107:
	ldr r9, [fp, #-60]
	ldr r10, [fp, #-116]
	cmp r9, r10
	blt main$L108
main$L109:
	ldr r9, [fp, #-60]
	sub r10, r9, #1
	str r10, [fp, #-84]
	ldr r9, [fp, #-40]
	mov r10, r9
	str r10, [fp, #-104]
	ldr r9, [fp, #-40]
	ldr r10, [r9]
	str r10, [fp, #-88]
	movw r0, #0
	ldr r9, [fp, #-84]
	cmp r9, r0
	bge main$L114
main$L113:
	movw r0, #65535
	movt r0, #65535
	bl exit
main$L114:
	ldr r9, [fp, #-84]
	ldr r10, [fp, #-88]
	cmp r9, r10
	bge main$L113
main$L115:
	ldr r9, [fp, #-84]
	add r0, r9, #1
	ldr r9, [fp, #-92]
	ldr r10, [fp, #-44]
	add r10, r9, r10
	str r10, [fp, #-100]
	ldr r9, [fp, #-108]
	ldr r10, [fp, #-44]
	add r10, r9, r10
	str r10, [fp, #-112]
	ldr r9, [fp, #-116]
	add r10, r9, #10
	str r10, [fp, #-120]
	ldr r9, [fp, #-140]
	ldr r10, [fp, #-56]
	add r10, r9, r10
	str r10, [fp, #-148]
	movw r1, #4
	mul r0, r0, r1
	ldr r9, [fp, #-104]
	ldr r0, [r9, r0]
	bl putint
	movw r0, #32
	bl putch
	ldr r9, [fp, #-76]
	mov r10, r9
	str r10, [fp, #-72]
	ldr r9, [fp, #-52]
	mov r2, r9
	ldr r9, [fp, #-100]
	mov r10, r9
	str r10, [fp, #-92]
	ldr r9, [fp, #-112]
	mov r10, r9
	str r10, [fp, #-108]
	ldr r9, [fp, #-120]
	mov r10, r9
	str r10, [fp, #-116]
	ldr r9, [fp, #-148]
	mov r10, r9
	str r10, [fp, #-140]
	b main$L102
main$L108:
	ldr r9, [fp, #-40]
	ldr r10, [r9]
	str r10, [fp, #-80]
	movw r0, #0
	ldr r9, [fp, #-60]
	cmp r9, r0
	bge main$L111
main$L110:
	movw r0, #65535
	movt r0, #65535
	bl exit
main$L111:
	ldr r9, [fp, #-60]
	ldr r10, [fp, #-80]
	cmp r9, r10
	bge main$L110
main$L112:
	ldr r9, [fp, #-136]
	ldr r10, [fp, #-40]
	str r9, [r10, r10]
	ldr r9, [fp, #-60]
	add r10, r9, #1
	str r10, [fp, #-60]
	ldr r9, [fp, #-128]
	add r10, r9, #4
	str r10, [fp, #-128]
	ldr r9, [fp, #-136]
	ldr r10, [fp, #-52]
	add r10, r9, r10
	str r10, [fp, #-136]
	ldr r9, [fp, #-60]
	mov r10, r9
	str r10, [fp, #-60]
	ldr r9, [fp, #-128]
	mov r10, r9
	str r10, [fp, #-128]
	ldr r9, [fp, #-136]
	mov r10, r9
	str r10, [fp, #-136]
	b main$L107

.balign 4
.global C$a
.section .text
.arm
C$a:
C$a$L100:
	push {r4-r10, fp, lr}
	sub sp, sp, #8
	add fp, sp, #40
	mov r10, r1
	str r10, [fp, #-40]
	ldr r9, [fp, #-40]
	add r0, r9, #1
	movw r1, #4
	mul r0, r0, r1
	bl malloc
	ldr r9, [fp, #-40]
	str r9, [r0]
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
