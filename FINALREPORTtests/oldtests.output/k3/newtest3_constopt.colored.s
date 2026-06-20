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
	sub sp, sp, #12
	add fp, sp, #44
	movw r0, #16
	bl malloc
	mov r10, r0
	str r10, [fp, #-40]
	ldr r9, [fp, #-44]
	mov r10, r9
	str r10, [fp, #-44]
	movw r0, #3
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
	movw r0, #44
	bl malloc
	movw r1, #10
	str r1, [r0]
	ldr r10, [fp, #-44]
	str r0, [r10]
	ldr r9, [fp, #-40]
	mov r0, r9
	ldr r9, [fp, #-40]
	ldr r1, [r9]
main$L101:
	movw r2, #16960
	movt r2, #15
	cmp r2, r1
	bge main$L100
main$L102:
	movw r1, #2308
	movt r1, #61
	ldr r0, [r0, r1]
	sub sp, fp, #44
	add sp, sp, #12
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
