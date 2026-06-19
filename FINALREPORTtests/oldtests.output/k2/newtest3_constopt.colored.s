.balign 4
.global TestClass$test1
.section .text
.arm
TestClass$test1:
TestClass$test1$L100:
	push {r4-r10, fp, lr}
	sub sp, sp, #4
	add fp, sp, #36
TestClass$test1$L101:
	movw r0, #0
	sub sp, fp, #36
	add sp, sp, #4
	pop {r4-r10, fp, lr}
	bx lr

.balign 4
.global main
.section .text
.arm
main:
main$L103:
	push {r4-r10, fp, lr}
	sub sp, sp, #20
	add fp, sp, #52
	movw r0, #16
	bl malloc
	mov r10, r0
	str r10, [fp, #-44]
	ldr r9, [fp, #-40]
	mov r10, r9
	str r10, [fp, #-52]
	movw r0, #3
	ldr r10, [fp, #-44]
	str r0, [r10]
	movw r0, #1
	ldr r10, [fp, #-44]
	str r0, [r10, #4]
	movw r0, #2
	ldr r10, [fp, #-44]
	str r0, [r10, #8]
	movw r0, #3
	ldr r10, [fp, #-44]
	str r0, [r10, #12]
	movw r0, #44
	bl malloc
	movw r1, #10
	str r1, [r0]
	ldr r10, [fp, #-52]
	str r0, [r10]
	ldr r9, [fp, #-44]
	mov r10, r9
	str r10, [fp, #-48]
	ldr r9, [fp, #-44]
	ldr r0, [r9]
main$L101:
	movw r1, #16960
	movt r1, #15
	cmp r1, r0
	bge main$L100
main$L102:
	movw r0, #2308
	movt r0, #61
	ldr r9, [fp, #-48]
	ldr r0, [r9, r0]
	sub sp, fp, #52
	add sp, sp, #20
	pop {r4-r10, fp, lr}
	bx lr
main$L100:
	movw r0, #65535
	movt r0, #65535
	bl exit

.global malloc
.global getint
.global getch
.global getarray
.global putint
.global putch
.global putarray
.global starttime
.global stoptime
