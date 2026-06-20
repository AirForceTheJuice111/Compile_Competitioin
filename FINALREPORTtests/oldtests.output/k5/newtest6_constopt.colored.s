.balign 4
.global main
.section .text
.arm
main:
main$L103:
	push {r4-r10, fp, lr}
	sub sp, sp, #8
	add fp, sp, #40
	movw r0, #16
	bl malloc
	mov r4, r0
	ldr r9, [fp, #-40]
	mov r10, r9
	str r10, [fp, #-40]
	movw r0, #3
	str r0, [r4]
	movw r0, #1
	str r0, [r4, #4]
	movw r0, #2
	str r0, [r4, #8]
	movw r0, #3
	str r0, [r4, #12]
	movw r0, #44
	bl malloc
	movw r1, #10
	str r1, [r0]
	ldr r10, [fp, #-40]
	str r0, [r10]
	ldr r0, [r4]
main$L101:
	movw r1, #16960
	movt r1, #15
	cmp r1, r0
	bge main$L100
main$L102:
	movw r0, #2308
	movt r0, #61
	ldr r0, [r4, r0]
	sub sp, fp, #40
	add sp, sp, #8
	pop {r4-r10, fp, lr}
	bx lr
main$L100:
	movw r0, #65535
	movt r0, #65535
	bl exit

.balign 4
.global TestClass$test0
.section .text
.arm
TestClass$test0:
TestClass$test0$L100:
	push {r4-r10, fp, lr}
	sub sp, sp, #4
	add fp, sp, #36
	mov r4, r0
	movw r0, #8
	bl malloc
	mov r1, r4
	movw r2, #1
	str r2, [r0]
	str r0, [r4]
	ldr r0, [r1]
	sub sp, fp, #36
	add sp, sp, #4
	pop {r4-r10, fp, lr}
	bx lr

.balign 4
.global TestClass$test1
.section .text
.arm
TestClass$test1:
TestClass$test1$L103:
	push {r4-r10, fp, lr}
	sub sp, sp, #4
	add fp, sp, #36
	mov r4, r0
	movw r0, #8
	bl malloc
	mov r1, r4
	movw r2, #1
	str r2, [r0]
	str r0, [r4]
	ldr r0, [r1]
	ldr r1, [r0]
TestClass$test1$L101:
	movw r2, #0
	cmp r2, r1
	bge TestClass$test1$L100
TestClass$test1$L102:
	ldr r0, [r0, #4]
	sub sp, fp, #36
	add sp, sp, #4
	pop {r4-r10, fp, lr}
	bx lr
TestClass$test1$L100:
	movw r0, #65535
	movt r0, #65535
	bl exit

.balign 4
.global TestClass$test2
.section .text
.arm
TestClass$test2:
TestClass$test2$L103:
	push {r4-r10, fp, lr}
	sub sp, sp, #8
	add fp, sp, #40
	mov r4, r0
	mov r10, r1
	str r10, [fp, #-40]
	movw r0, #8
	bl malloc
	movw r1, #1
	str r1, [r0]
	ldr r1, [r0]
TestClass$test2$L101:
	movw r2, #0
	cmp r2, r1
	bge TestClass$test2$L100
TestClass$test2$L102:
	ldr r9, [fp, #-40]
	str r9, [r0, #4]
	ldr r0, [r4]
	sub sp, fp, #40
	add sp, sp, #8
	pop {r4-r10, fp, lr}
	bx lr
TestClass$test2$L100:
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
