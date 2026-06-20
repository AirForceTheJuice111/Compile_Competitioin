.balign 4
.global C$m
.section .text
.arm
C$m:
C$m$L100:
	push {r4-r10, fp, lr}
	sub sp, sp, #4
	add fp, sp, #36
	ldr r0, [r0]
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
	movw r0, #8
	bl malloc
	mov r10, r0
	str r10, [fp, #-52]
	mov r10, #3
	str r10, [fp, #-44]
	movw r0, #20
	bl malloc
	mov r10, r0
	str r10, [fp, #-56]
	movw r0, #4
	ldr r10, [fp, #-56]
	str r0, [r10]
	ldr r9, [fp, #-52]
	mov r1, r9
	ldr r9, [fp, #-52]
	add r10, r9, #4
	str r10, [fp, #-68]
	ldr r9, [fp, #-52]
	mov r10, r9
	str r10, [fp, #-60]
	ldr r9, [fp, #-44]
	sub r10, r9, #1
	str r10, [fp, #-80]
	movw r0, #1
	ldr r10, [fp, #-56]
	str r0, [r10, #4]
	movw r0, #2
	ldr r10, [fp, #-56]
	str r0, [r10, #8]
	movw r0, #3
	ldr r10, [fp, #-56]
	str r0, [r10, #12]
	movw r0, #4
	ldr r10, [fp, #-56]
	str r0, [r10, #16]
	ldr r9, [fp, #-56]
	str r9, [r1]
	ldr r0, =C$m
	ldr r10, [fp, #-68]
	str r0, [r10]
	ldr r9, [fp, #-60]
	ldr r1, [r9, #4]
	ldr r9, [fp, #-60]
	mov r0, r9
	blx r1
	mov r1, r0
	movw r0, #4
	ldr r9, [fp, #-80]
	mul r0, r9, r0
	mov r10, r1
	str r10, [fp, #-40]
	add r0, r0, #4
	ldr r9, [fp, #-44]
	mov r1, r9
	mov r10, r0
	str r10, [fp, #-72]
main$L102:
	movw r0, #0
	ldr r9, [fp, #-72]
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
	str r10, [fp, #-48]
	ldr r9, [fp, #-40]
	mov r10, r9
	str r10, [fp, #-40]
	ldr r9, [fp, #-40]
	ldr r10, [r9]
	str r10, [fp, #-64]
	movw r0, #0
	ldr r9, [fp, #-48]
	cmp r9, r0
	bge main$L106
main$L105:
	movw r0, #65535
	movt r0, #65535
	bl exit
main$L106:
	ldr r9, [fp, #-48]
	ldr r10, [fp, #-64]
	cmp r9, r10
	bge main$L105
main$L107:
	ldr r9, [fp, #-40]
	ldr r10, [fp, #-72]
	ldr r0, [r9, r10]
	ldr r9, [fp, #-72]
	sub r10, r9, #4
	str r10, [fp, #-76]
	bl putint
	movw r0, #32
	bl putch
	ldr r9, [fp, #-48]
	mov r1, r9
	ldr r9, [fp, #-76]
	mov r10, r9
	str r10, [fp, #-72]
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
