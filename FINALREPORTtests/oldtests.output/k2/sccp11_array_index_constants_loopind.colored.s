.balign 4
.global main
.section .text
.arm
main:
main$L106:
	push {r4-r10, fp, lr}
	sub sp, sp, #28
	add fp, sp, #60
	movw r0, #20
	bl malloc
	mov r10, r0
	str r10, [fp, #-40]
	movw r1, #1
	movw r0, #1
	add r10, r1, r0
	str r10, [fp, #-44]
	movw r0, #4
	ldr r10, [fp, #-40]
	str r0, [r10]
	movw r0, #1
	ldr r10, [fp, #-40]
	str r0, [r10, #4]
	movw r0, #2
	ldr r10, [fp, #-40]
	str r0, [r10, #8]
	movw r0, #3
	ldr r10, [fp, #-40]
	str r0, [r10, #12]
	movw r0, #4
	ldr r10, [fp, #-40]
	str r0, [r10, #16]
	ldr r9, [fp, #-40]
	mov r10, r9
	str r10, [fp, #-56]
	ldr r9, [fp, #-40]
	ldr r10, [r9]
	str r10, [fp, #-48]
	movw r0, #0
	ldr r9, [fp, #-44]
	cmp r9, r0
	bge main$L101
main$L100:
	movw r0, #65535
	movt r0, #65535
	bl exit
main$L101:
	ldr r9, [fp, #-44]
	ldr r10, [fp, #-48]
	cmp r9, r10
	bge main$L100
main$L102:
	ldr r9, [fp, #-44]
	add r0, r9, #1
	ldr r9, [fp, #-40]
	mov r10, r9
	str r10, [fp, #-60]
	movw r1, #4
	mul r0, r0, r1
	movw r1, #7
	ldr r10, [fp, #-56]
	str r1, [r10, r0]
	ldr r9, [fp, #-40]
	ldr r10, [r9]
	str r10, [fp, #-52]
	movw r1, #2
	movw r0, #0
	cmp r1, r0
	bge main$L104
main$L103:
	movw r0, #65535
	movt r0, #65535
	bl exit
main$L104:
	movw r0, #2
	ldr r10, [fp, #-52]
	cmp r0, r10
	bge main$L103
main$L105:
	movw r1, #2
	movw r0, #1
	add r0, r1, r0
	movw r1, #4
	mul r0, r0, r1
	ldr r9, [fp, #-60]
	ldr r0, [r9, r0]
	sub sp, fp, #60
	add sp, sp, #28
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
