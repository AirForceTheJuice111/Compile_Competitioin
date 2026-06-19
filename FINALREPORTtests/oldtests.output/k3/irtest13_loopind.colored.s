.balign 4
.global main
.section .text
.arm
main:
main$L103:
	push {r4-r10, fp, lr}
	sub sp, sp, #16
	add fp, sp, #48
	mov r10, #1
	str r10, [fp, #-40]
	ldr r9, [fp, #-40]
	add r0, r9, #1
	movw r1, #4
	mul r0, r0, r1
	bl malloc
	ldr r9, [fp, #-40]
	str r9, [r0]
	ldr r10, [r0]
	str r10, [fp, #-44]
	mov r10, r0
	str r10, [fp, #-48]
	movw r1, #0
	movw r0, #0
	cmp r1, r0
	bge main$L101
main$L100:
	movw r0, #65535
	movt r0, #65535
	bl exit
main$L101:
	movw r0, #0
	ldr r10, [fp, #-44]
	cmp r0, r10
	bge main$L100
main$L102:
	movw r1, #0
	movw r0, #1
	add r0, r1, r0
	movw r1, #4
	mul r0, r0, r1
	movw r1, #0
	ldr r10, [fp, #-48]
	str r1, [r10, r0]
main$L104:
	movw r0, #0
	sub sp, fp, #48
	add sp, sp, #16
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
