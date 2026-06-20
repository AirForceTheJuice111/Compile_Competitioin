.balign 4
.global main
.section .text
.arm
main:
main$L108:
	push {r4-r10, fp, lr}
	sub sp, sp, #40
	add fp, sp, #72
	movw r0, #12
	bl malloc
	mov r10, r0
	str r10, [fp, #-44]
	movw r0, #20
	bl malloc
	mov r2, r0
	movw r0, #4
	str r0, [r2]
	ldr r9, [fp, #-44]
	mov r0, r9
	ldr r9, [fp, #-44]
	add r10, r9, #4
	str r10, [fp, #-68]
	ldr r9, [fp, #-44]
	add r10, r9, #8
	str r10, [fp, #-72]
	ldr r9, [fp, #-44]
	mov r10, r9
	str r10, [fp, #-48]
	movw r1, #1
	str r1, [r2, #4]
	movw r1, #2
	str r1, [r2, #8]
	movw r1, #3
	str r1, [r2, #12]
	movw r1, #4
	str r1, [r2, #16]
	str r2, [r0]
	movw r0, #24
	bl malloc
	mov r2, r0
	movw r0, #5
	str r0, [r2]
	ldr r9, [fp, #-48]
	add r0, r9, #8
	ldr r9, [fp, #-48]
	mov r10, r9
	str r10, [fp, #-60]
	movw r1, #5
	str r1, [r2, #4]
	movw r1, #6
	str r1, [r2, #8]
	movw r1, #7
	str r1, [r2, #12]
	movw r1, #8
	str r1, [r2, #16]
	movw r1, #9
	str r1, [r2, #20]
	ldr r10, [fp, #-68]
	str r2, [r10]
	ldr r1, =C$m
	ldr r10, [fp, #-72]
	str r1, [r10]
	ldr r10, [r0]
	str r10, [fp, #-56]
	bl getint
	mov r1, r0
	ldr r9, [fp, #-56]
	mov r10, r9
	str r10, [fp, #-56]
	ldr r9, [fp, #-60]
	mov r0, r9
	ldr r9, [fp, #-56]
	blx r9
	mov r10, r0
	str r10, [fp, #-64]
	ldr r9, [fp, #-64]
	ldr r0, [r9]
	ldr r9, [fp, #-64]
	mov r10, r9
	str r10, [fp, #-64]
main$L102:
	movw r1, #0
	cmp r0, r1
	bge main$L103
main$L104:
	movw r0, #10
	bl putch
	movw r0, #2
	sub sp, fp, #72
	add sp, sp, #40
	pop {r4-r10, fp, lr}
	bx lr
main$L103:
	sub r10, r0, #1
	str r10, [fp, #-40]
	ldr r9, [fp, #-64]
	ldr r10, [r9]
	str r10, [fp, #-52]
	movw r0, #0
	ldr r9, [fp, #-40]
	cmp r9, r0
	bge main$L106
main$L105:
	movw r0, #65535
	movt r0, #65535
	bl exit
main$L106:
	ldr r9, [fp, #-40]
	ldr r10, [fp, #-52]
	cmp r9, r10
	bge main$L105
main$L107:
	ldr r9, [fp, #-40]
	add r0, r9, #1
	movw r1, #4
	mul r0, r0, r1
	ldr r9, [fp, #-64]
	ldr r0, [r9, r0]
	bl putint
	movw r0, #32
	bl putch
	ldr r9, [fp, #-40]
	mov r0, r9
	b main$L102

.balign 4
.global C$m
.section .text
.arm
C$m:
C$m$L105:
	push {r4-r10, fp, lr}
	sub sp, sp, #4
	add fp, sp, #36
	mov r2, r0
	movw r0, #0
	cmp r1, r0
	bgt C$m$L102
C$m$L103:
	ldr r0, [r2, #4]
	sub sp, fp, #36
	add sp, sp, #4
	pop {r4-r10, fp, lr}
	bx lr
C$m$L102:
	ldr r0, [r2]
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
