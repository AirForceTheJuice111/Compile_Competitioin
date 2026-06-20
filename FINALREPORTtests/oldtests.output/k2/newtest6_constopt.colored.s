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

.balign 4
.global TestClass$test0
.section .text
.arm
TestClass$test0:
TestClass$test0$L100:
	push {r4-r10, fp, lr}
	sub sp, sp, #16
	add fp, sp, #48
	mov r10, r0
	str r10, [fp, #-40]
	movw r0, #8
	bl malloc
	mov r10, r0
	str r10, [fp, #-48]
	ldr r9, [fp, #-40]
	mov r1, r9
	ldr r9, [fp, #-40]
	mov r0, r9
	movw r10, #1
	str r10, [fp, #-44]
	ldr r9, [fp, #-44]
	ldr r10, [fp, #-48]
	str r9, [r10]
	ldr r9, [fp, #-48]
	str r9, [r1]
	ldr r0, [r0]
	sub sp, fp, #48
	add sp, sp, #16
	pop {r4-r10, fp, lr}
	bx lr

.balign 4
.global TestClass$test1
.section .text
.arm
TestClass$test1:
TestClass$test1$L103:
	push {r4-r10, fp, lr}
	sub sp, sp, #20
	add fp, sp, #52
	mov r10, r0
	str r10, [fp, #-40]
	movw r0, #8
	bl malloc
	mov r10, r0
	str r10, [fp, #-52]
	ldr r9, [fp, #-40]
	mov r1, r9
	ldr r9, [fp, #-40]
	mov r0, r9
	movw r10, #1
	str r10, [fp, #-44]
	ldr r9, [fp, #-44]
	ldr r10, [fp, #-52]
	str r9, [r10]
	ldr r9, [fp, #-52]
	str r9, [r1]
	ldr r0, [r0]
	ldr r1, [r0]
TestClass$test1$L101:
	movw r10, #0
	str r10, [fp, #-48]
	ldr r9, [fp, #-48]
	cmp r9, r1
	bge TestClass$test1$L100
TestClass$test1$L102:
	ldr r0, [r0, #4]
	sub sp, fp, #52
	add sp, sp, #20
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
	sub sp, sp, #16
	add fp, sp, #48
	mov r10, r0
	str r10, [fp, #-44]
	mov r10, r1
	str r10, [fp, #-40]
	movw r0, #8
	bl malloc
	movw r1, #1
	str r1, [r0]
	ldr r1, [r0]
TestClass$test2$L101:
	movw r10, #0
	str r10, [fp, #-48]
	ldr r9, [fp, #-48]
	cmp r9, r1
	bge TestClass$test2$L100
TestClass$test2$L102:
	ldr r9, [fp, #-40]
	str r9, [r0, #4]
	ldr r9, [fp, #-44]
	ldr r0, [r9]
	sub sp, fp, #48
	add sp, sp, #16
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
