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

.balign 4
.global main
.section .text
.arm
main:
main$L108:
	push {r4-r10, fp, lr}
	sub sp, sp, #48
	add fp, sp, #80
	movw r0, #12
	bl malloc
	mov r10, r0
	str r10, [fp, #-48]
	movw r0, #20
	bl malloc
	mov r2, r0
	movw r0, #4
	str r0, [r2]
	ldr r9, [fp, #-48]
	mov r0, r9
	ldr r9, [fp, #-48]
	add r10, r9, #4
	str r10, [fp, #-68]
	ldr r9, [fp, #-48]
	add r10, r9, #8
	str r10, [fp, #-72]
	ldr r9, [fp, #-48]
	mov r10, r9
	str r10, [fp, #-52]
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
	ldr r9, [fp, #-52]
	add r0, r9, #8
	ldr r9, [fp, #-52]
	mov r10, r9
	str r10, [fp, #-64]
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
	str r10, [fp, #-60]
	bl getint
	mov r1, r0
	ldr r9, [fp, #-60]
	mov r10, r9
	str r10, [fp, #-60]
	ldr r9, [fp, #-64]
	mov r0, r9
	ldr r9, [fp, #-60]
	blx r9
	mov r10, r0
	str r10, [fp, #-40]
	ldr r9, [fp, #-40]
	ldr r1, [r9]
	sub r0, r1, #1
	movw r2, #4
	mul r0, r0, r2
	add r0, r0, #4
	mov r10, r0
	str r10, [fp, #-76]
main$L102:
	movw r0, #0
	ldr r9, [fp, #-76]
	cmp r9, r0
	bge main$L103
main$L104:
	movw r0, #10
	bl putch
	movw r0, #2
	sub sp, fp, #80
	add sp, sp, #48
	pop {r4-r10, fp, lr}
	bx lr
main$L103:
	sub r10, r1, #1
	str r10, [fp, #-44]
	ldr r9, [fp, #-40]
	mov r10, r9
	str r10, [fp, #-40]
	ldr r9, [fp, #-40]
	ldr r10, [r9]
	str r10, [fp, #-56]
	movw r0, #0
	ldr r9, [fp, #-44]
	cmp r9, r0
	bge main$L106
main$L105:
	movw r0, #65535
	movt r0, #65535
	bl exit
main$L106:
	ldr r9, [fp, #-44]
	ldr r10, [fp, #-56]
	cmp r9, r10
	bge main$L105
main$L107:
	ldr r9, [fp, #-40]
	ldr r10, [fp, #-76]
	ldr r0, [r9, r10]
	ldr r9, [fp, #-76]
	sub r10, r9, #4
	str r10, [fp, #-80]
	bl putint
	movw r0, #32
	bl putch
	ldr r9, [fp, #-44]
	mov r1, r9
	ldr r9, [fp, #-80]
	mov r10, r9
	str r10, [fp, #-76]
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
