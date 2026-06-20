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
	mov r10, r0
	str r10, [fp, #-52]
	movw r0, #4
	ldr r10, [fp, #-52]
	str r0, [r10]
	ldr r9, [fp, #-48]
	mov r1, r9
	ldr r9, [fp, #-48]
	add r10, r9, #4
	str r10, [fp, #-76]
	ldr r9, [fp, #-48]
	add r10, r9, #8
	str r10, [fp, #-80]
	ldr r9, [fp, #-48]
	mov r10, r9
	str r10, [fp, #-60]
	movw r0, #1
	ldr r10, [fp, #-52]
	str r0, [r10, #4]
	movw r0, #2
	ldr r10, [fp, #-52]
	str r0, [r10, #8]
	movw r0, #3
	ldr r10, [fp, #-52]
	str r0, [r10, #12]
	movw r0, #4
	ldr r10, [fp, #-52]
	str r0, [r10, #16]
	ldr r9, [fp, #-52]
	str r9, [r1]
	movw r0, #24
	bl malloc
	mov r10, r0
	str r10, [fp, #-56]
	movw r0, #5
	ldr r10, [fp, #-56]
	str r0, [r10]
	ldr r9, [fp, #-60]
	add r1, r9, #8
	ldr r9, [fp, #-60]
	mov r10, r9
	str r10, [fp, #-72]
	movw r0, #5
	ldr r10, [fp, #-56]
	str r0, [r10, #4]
	movw r0, #6
	ldr r10, [fp, #-56]
	str r0, [r10, #8]
	movw r0, #7
	ldr r10, [fp, #-56]
	str r0, [r10, #12]
	movw r0, #8
	ldr r10, [fp, #-56]
	str r0, [r10, #16]
	movw r0, #9
	ldr r10, [fp, #-56]
	str r0, [r10, #20]
	ldr r9, [fp, #-56]
	ldr r10, [fp, #-76]
	str r9, [r10]
	ldr r0, =C$m
	ldr r10, [fp, #-80]
	str r0, [r10]
	ldr r10, [r1]
	str r10, [fp, #-68]
	bl getint
	mov r1, r0
	ldr r9, [fp, #-68]
	mov r10, r9
	str r10, [fp, #-68]
	ldr r9, [fp, #-72]
	mov r0, r9
	ldr r9, [fp, #-68]
	blx r9
	mov r10, r0
	str r10, [fp, #-40]
	ldr r9, [fp, #-40]
	ldr r0, [r9]
main$L102:
	movw r1, #0
	cmp r0, r1
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
	sub r10, r0, #1
	str r10, [fp, #-44]
	ldr r9, [fp, #-40]
	mov r10, r9
	str r10, [fp, #-40]
	ldr r9, [fp, #-40]
	ldr r10, [r9]
	str r10, [fp, #-64]
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
	ldr r10, [fp, #-64]
	cmp r9, r10
	bge main$L105
main$L107:
	ldr r9, [fp, #-44]
	add r0, r9, #1
	movw r1, #4
	mul r0, r0, r1
	ldr r9, [fp, #-40]
	ldr r0, [r9, r0]
	bl putint
	movw r0, #32
	bl putch
	ldr r9, [fp, #-44]
	mov r0, r9
	b main$L102

.balign 4
.global C$m
.section .text
.arm
C$m:
C$m$L105:
	push {r4-r10, fp, lr}
	sub sp, sp, #8
	add fp, sp, #40
	mov r10, r0
	str r10, [fp, #-40]
	movw r0, #0
	cmp r1, r0
	bgt C$m$L102
C$m$L103:
	ldr r9, [fp, #-40]
	ldr r0, [r9, #4]
	sub sp, fp, #40
	add sp, sp, #8
	pop {r4-r10, fp, lr}
	bx lr
C$m$L102:
	ldr r9, [fp, #-40]
	ldr r0, [r9]
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
