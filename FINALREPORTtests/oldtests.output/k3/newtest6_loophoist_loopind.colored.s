.balign 4
.global TestClass$test1
.section .text
.arm
TestClass$test1:
TestClass$test1$L103:
	push {r4-r10, fp, lr}
	sub sp, sp, #24
	add fp, sp, #56
	mov r10, #1
	str r10, [fp, #-40]
	mov r10, r0
	str r10, [fp, #-52]
	mov r10, r0
	str r10, [fp, #-56]
	ldr r9, [fp, #-40]
	add r0, r9, #1
	movw r1, #4
	mul r0, r0, r1
	bl malloc
	ldr r9, [fp, #-40]
	str r9, [r0]
	ldr r10, [fp, #-52]
	str r0, [r10]
	ldr r9, [fp, #-56]
	ldr r10, [r9]
	str r10, [fp, #-48]
	ldr r9, [fp, #-48]
	ldr r10, [r9]
	str r10, [fp, #-44]
	ldr r9, [fp, #-48]
	mov r10, r9
	str r10, [fp, #-48]
	movw r1, #0
	movw r0, #0
	cmp r1, r0
	bge TestClass$test1$L101
TestClass$test1$L100:
	movw r0, #65535
	movt r0, #65535
	bl exit
TestClass$test1$L101:
	movw r0, #0
	ldr r10, [fp, #-44]
	cmp r0, r10
	bge TestClass$test1$L100
TestClass$test1$L102:
	movw r1, #0
	movw r0, #1
	add r0, r1, r0
	movw r1, #4
	mul r0, r0, r1
	ldr r9, [fp, #-48]
	ldr r0, [r9, r0]
	sub sp, fp, #56
	add sp, sp, #24
	pop {r4-r10, fp, lr}
	bx lr

.balign 4
.global TestClass$test2
.section .text
.arm
TestClass$test2:
TestClass$test2$L103:
	push {r4-r10, fp, lr}
	sub sp, sp, #24
	add fp, sp, #56
	mov r10, r0
	str r10, [fp, #-44]
	mov r10, r1
	str r10, [fp, #-40]
	mov r10, #1
	str r10, [fp, #-48]
	ldr r9, [fp, #-48]
	add r0, r9, #1
	movw r1, #4
	mul r0, r0, r1
	bl malloc
	ldr r9, [fp, #-48]
	str r9, [r0]
	ldr r10, [r0]
	str r10, [fp, #-52]
	mov r10, r0
	str r10, [fp, #-56]
	movw r1, #0
	movw r0, #0
	cmp r1, r0
	bge TestClass$test2$L101
TestClass$test2$L100:
	movw r0, #65535
	movt r0, #65535
	bl exit
TestClass$test2$L101:
	movw r0, #0
	ldr r10, [fp, #-52]
	cmp r0, r10
	bge TestClass$test2$L100
TestClass$test2$L102:
	movw r1, #0
	movw r0, #1
	add r1, r1, r0
	ldr r9, [fp, #-44]
	mov r0, r9
	movw r2, #4
	mul r1, r1, r2
	ldr r9, [fp, #-40]
	ldr r10, [fp, #-56]
	str r9, [r10, r1]
	ldr r0, [r0]
	sub sp, fp, #56
	add sp, sp, #24
	pop {r4-r10, fp, lr}
	bx lr

.balign 4
.global TestClass$test0
.section .text
.arm
TestClass$test0:
TestClass$test0$L100:
	push {r4-r10, fp, lr}
	sub sp, sp, #16
	add fp, sp, #48
	mov r10, #1
	str r10, [fp, #-40]
	mov r10, r0
	str r10, [fp, #-44]
	mov r10, r0
	str r10, [fp, #-48]
	ldr r9, [fp, #-40]
	add r0, r9, #1
	movw r1, #4
	mul r0, r0, r1
	bl malloc
	ldr r9, [fp, #-40]
	str r9, [r0]
	ldr r10, [fp, #-44]
	str r0, [r10]
	ldr r9, [fp, #-48]
	ldr r0, [r9]
	sub sp, fp, #48
	add sp, sp, #16
	pop {r4-r10, fp, lr}
	bx lr

.balign 4
.global main
.section .text
.arm
main:
main$L103:
	push {r4-r10, fp, lr}
	sub sp, sp, #24
	add fp, sp, #56
	movw r0, #16
	bl malloc
	mov r10, r0
	str r10, [fp, #-40]
	mov r10, #10
	str r10, [fp, #-44]
	ldr r9, [fp, #-56]
	mov r10, r9
	str r10, [fp, #-56]
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
	ldr r9, [fp, #-40]
	mov r10, r9
	str r10, [fp, #-52]
	ldr r9, [fp, #-44]
	add r0, r9, #1
	movw r1, #4
	mul r0, r0, r1
	bl malloc
	ldr r9, [fp, #-44]
	str r9, [r0]
	ldr r10, [fp, #-56]
	str r0, [r10]
	ldr r9, [fp, #-40]
	ldr r10, [r9]
	str r10, [fp, #-48]
	movw r1, #16960
	movt r1, #15
	movw r0, #0
	cmp r1, r0
	bge main$L101
main$L100:
	movw r0, #65535
	movt r0, #65535
	bl exit
main$L101:
	movw r0, #16960
	movt r0, #15
	ldr r10, [fp, #-48]
	cmp r0, r10
	bge main$L100
main$L102:
	movw r1, #16960
	movt r1, #15
	movw r0, #1
	add r0, r1, r0
	movw r1, #4
	mul r0, r0, r1
	ldr r9, [fp, #-52]
	ldr r0, [r9, r0]
	sub sp, fp, #56
	add sp, sp, #24
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
